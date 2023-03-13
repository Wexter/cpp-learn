#include <typeinfo>
#include <type_traits>
#include "util.h"
#include <type_traits>
#include <memory>
#include <vector>
#include <map>
#include <cassert>
#include <sstream>

static size_t allocated_memory = 0;

static bool log_allocations = false;

template <typename T>
struct ScopeHandler {
    T &obj_ptr;

    ScopeHandler(T& obj) :
        obj_ptr(obj)
    {
        obj_ptr = true;
    }

    ~ScopeHandler()
    {
        obj_ptr = false;
    }
};

const char * byte_size_str(uint64_t size)
{
    const char *suffix[] = {"bytes", "Kb", "Mb", "Gb", "Tb"};

	char length = sizeof(suffix) / sizeof(suffix[0]);

	int i = 0;
	double dblBytes = size;

	if (size > 0) {
        for (; (dblBytes / 1024.) > 1 && i < length - 1; i++)
            dblBytes /= 1024.0;
    }

	static char output[200];
	sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
	return output;
}

void print_memory_usage()
{
    PRINTLN("Memory used: " << byte_size_str(allocated_memory));
}

void* operator new(size_t size)
{
    allocated_memory += size;
    LOG_DEBUG_IF(log_allocations, "Allocated: " << byte_size_str(size) << " Total: " << byte_size_str(allocated_memory));
    return malloc(size);
}

void operator delete(void* p, size_t size)
{
    LOG_DEBUG_IF(size > allocated_memory, "Used: " << allocated_memory << " Free: " << size);

    assert(size <= allocated_memory);

    allocated_memory -= size;

    LOG_DEBUG_IF(log_allocations, "Deallocated: " << /* byte_size_str */(size) << " Total: " << /* byte_size_str */(allocated_memory));

    free(p);
}

class NameComponent {
    friend std::ostream& operator<<(std::ostream& out, NameComponent& rhs);
public:
    std::string name;

    NameComponent(std::string name) :
        name(name)
    {}

    ~NameComponent()
    {
        name.clear();
    }
};

class PositionComponent {
public:
    int x;
    int y;

    PositionComponent(int x, int y) :
        x(x), y(y)
    {
        // LOG_DEBUG("PositionComponent(" << x << ", " << y << ")");
    }

    ~PositionComponent()
    {
        x = -1;
        y = -1;
        // LOG_DEBUG("~PositionComponent(" << x << ", " << y << ")");
    }
};

class Test1Component {
    int i;
public:
    Test1Component() :
        i(0) {}
    Test1Component(int i) :
        i(i) {}
};

class Test2Component {
    int i;
public:
    Test2Component() :
        i(0) {}
    Test2Component(int i) :
        i(i) {}
};

static std::map<std::size_t, const char *> type_name_map;

const char* get_type_name(std::size_t type_hash)
{
    assert(type_name_map.contains(type_hash));

    return type_name_map[type_hash];
}

template <typename T>
void try_add_type()
{
    std::size_t type_hash = typeid(T).hash_code();

    if (type_name_map.contains(type_hash))
        return;

    // ScopeHandler sh(allocated_memory);

    // LOG_DEBUG(allocated_memory);

    // LOG_DEBUG("Add new type: id = " << type_hash << " name = " << type_name);
    const char* type_name = get_type_name<T>();

    type_name_map.emplace(type_hash, type_name);

    size_t map_total_size = 0;

    for (auto & [key, val] : type_name_map)
    {
        map_total_size += sizeof(std::size_t) + sizeof(val);
    }

    LOG_DEBUG("Map type total size: " << byte_size_str(map_total_size));
}

class Entity {
    using base_class = std::shared_ptr<void>;
    using component_map = std::map<std::size_t, base_class>;
    using map_iterator = component_map::iterator;

    component_map __entity_components;

    friend std::ostream& operator<<(std::ostream& out, Entity& rhs);

public:

    template<typename T, typename... Args>
    T& add_component(Args&&... args)
    {
        try_add_type<T>();

        std::shared_ptr<T> new_component(std::make_shared<T>(std::forward<Args>(args)...));

        __entity_components.emplace(typeid(T).hash_code(), new_component);

        return *new_component;
    }

    template<typename T>
    T& get_component()
    {
        assert(has_component<T>());

        return *std::static_pointer_cast<T>(__entity_components[typeid(T).hash_code()]);
    }

    template<typename T>
    bool has_component()
    {
        return __entity_components.contains(typeid(T).hash_code());
    }

    void clear_components()
    {
        __entity_components.clear();
    }

    std::string components_str()
    {
        std::stringstream type_str;

        for (auto item : __entity_components)
        {
            // LOG_DEBUG(item.first);
            // LOG_DEBUG(get_type_name(item.first));
            type_str << get_type_name(item.first) << std::endl;
        }

        return type_str.str();
    }

    void print_components()
    {
        for (auto const & [key, val] : __entity_components)
        {
            PRINTLN(key);
        }
    }
};

std::ostream& operator<<(std::ostream& out, Entity& rhs)
{
    out << "Entity: " << std::endl << rhs.components_str();

    return out;
}

static std::vector<Entity> entities;

void print_main_menu()
{
    PRINTLN("-------------------------");
    PRINTLN("1: Create new entity");
    PRINTLN("2: Edit entity");
    PRINTLN("3: Delete entity");
    PRINTLN("4: Print entities");
    PRINTLN("5: Print memory usage");
    PRINTLN("0: Exit");
    PRINTLN("-------------------------");
}

void print_entities()
{
    PRINTLN("Entities:");

    for (size_t idx = 0; idx < entities.size(); idx++)
    {
        PRINTLN(idx << ": " << entities[idx]);
    }
}

void create_new_entity_menu()
{
    // ScopeHandler sh(allocated_memory);

    int offset = entities.size();
    for (int i = 0; i < 1000000; i++) {
        int idx = i + offset;
        entities.emplace_back();

        /* switch (idx % 3)
        {
        case 0:
            entities[idx].add_component<NameComponent>("Name " + std::to_string(idx));
            entities[idx].add_component<PositionComponent>(0, 0);
            break;
        case 1:
            entities[idx].add_component<NameComponent>("Name " + std::to_string(idx));
            break;
        case 2:
            entities[idx].add_component<PositionComponent>(0, 0);
            break;
        } */
        entities[idx].add_component<NameComponent>("Name " + std::to_string(idx));
        entities[idx].add_component<PositionComponent>(0, 0);
        entities[idx].add_component<Test1Component>();
        // entities[idx].add_component<Test2Component>();
    }

    print_memory_usage();
}

void print_entity_edit_menu()
{
    PRINTLN("1: Add component");
    // PRINTLN("2: Remove component");
    // PRINTLN("3: Remove all components");
    PRINTLN("0: Exit");
}

int get_component_id()
{
    PRINTLN("1: NameComponent");
    PRINTLN("2: PositionComponent");

    int component_id;

    std::cin >> component_id;

    return component_id;
}

void add_entity_component(int entity_idx, int component_id)
{
    Entity& entity = entities[entity_idx];

    switch(component_id)
    {
    case 1:
        entity.add_component<NameComponent>("Test");
        break;
    case 2:
        entity.add_component<PositionComponent>(0, 0);
        break;
    default:
        break;
    }
}

void edit_entity_menu()
{
    PRINTLN("Select entity:");
    print_entities();

    int entity_idx;

    std::cin >> entity_idx;

    bool exit = false;

    do {
        print_entity_edit_menu();

        int menu_item;
        std::cin >> menu_item;

        switch (menu_item)
        {
        case 0: // exit
            exit = true;
            break;
        case 1:
            add_entity_component(entity_idx, get_component_id());
            break;
        default:
            break;
        }
    } while (!exit);
}

void delete_entity_menu()
{
    PRINTLN("Remove entities");
    entities.clear();
    entities.shrink_to_fit();
    print_memory_usage();
}

int main() {
    // PRINTLN("sizeof(Entity): " << sizeof(Entity));
    // PRINTLN("sizeof(NameComponent): " << sizeof(NameComponent));
    // PRINTLN("sizeof(PositionComponent): " << sizeof(PositionComponent));
    // PRINTLN("sizeof(std::shared_ptr<void>)" << sizeof(std::shared_ptr<void>));
    PRINTLN("sizeof(std::map<std::size_t, std::shared_ptr<void>>) = " << sizeof(std::map<std::size_t, std::shared_ptr<void> >));

    bool exit = false;

    do {
        print_main_menu();

        int menu_item;

        std::cin >> menu_item;

        switch (menu_item)
        {
        case 0:
            exit = true;
            break;
        case 1:
            create_new_entity_menu();
            break;
        case 2:
            edit_entity_menu();
            break;
        case 3:
            delete_entity_menu();
            break;
        case 4:
            print_entities();
            break;
        case 5:
            print_memory_usage();
            break;
        case 6:
            type_name_map.clear();
            // type_name_map.shrink_to_fit();
            print_memory_usage();
            break;
        default:
            break;
        }
    } while (!exit);

    entities.clear();

    return 0;
}
