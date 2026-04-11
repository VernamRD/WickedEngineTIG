#include "lua.h"
#include "luacode.h"
#include "lualib.h"
#include <fstream>
#include <sstream>
#include <string>

static int io_readfile(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        lua_pushnil(L);
        lua_pushstring(L, "cannot open file");
        return 2;
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    std::string content = ss.str();
    lua_pushlstring(L, content.c_str(), content.size());
    return 1;
}

static int io_writefile(lua_State* L) {
    const char* path    = luaL_checkstring(L, 1);
    size_t len          = 0;
    const char* content = luaL_checklstring(L, 2, &len);
    std::ofstream f(path, std::ios::binary);
    if (!f) {
        lua_pushnil(L);
        lua_pushstring(L, "cannot open file for writing");
        return 2;
    }
    f.write(content, static_cast<std::streamsize>(len));
    lua_pushboolean(L, 1);
    return 1;
}

static int io_appendfile(lua_State* L) {
    const char* path    = luaL_checkstring(L, 1);
    size_t len          = 0;
    const char* content = luaL_checklstring(L, 2, &len);
    std::ofstream f(path, std::ios::binary | std::ios::app);
    if (!f) {
        lua_pushnil(L);
        lua_pushstring(L, "cannot open file for appending");
        return 2;
    }
    f.write(content, static_cast<std::streamsize>(len));
    lua_pushboolean(L, 1);
    return 1;
}

static const luaL_Reg io_lib[] = {
    {"readfile",   io_readfile},
    {"writefile",  io_writefile},
    {"appendfile", io_appendfile},
    {nullptr, nullptr}
};

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: luau_host <script.luau> [args...]\n");
        return 1;
    }

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // Регистрируем io
    lua_newtable(L);
    luaL_register(L, nullptr, io_lib);
    lua_setglobal(L, "io");

    // Передаём все аргументы после имени скрипта в глобальную таблицу arg
    // arg[0] = script path, arg[1] = первый аргумент, и т.д.
    lua_newtable(L);
    lua_pushstring(L, argv[1]);
    lua_rawseti(L, -2, 0);
    for (int i = 2; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i - 1); // arg[1], arg[2], ...
    }
    lua_setglobal(L, "arg");

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        printf("Cannot open script: %s\n", argv[1]);
        lua_close(L);
        return 1;
    }
    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    size_t bytecodeSize = 0;
    char* bytecode = luau_compile(source.c_str(), source.length(), nullptr, &bytecodeSize);

    if (luau_load(L, argv[1], bytecode, bytecodeSize, 0) != LUA_OK) {
        printf("Error loading: %s\n", lua_tostring(L, -1));
        free(bytecode);
        lua_close(L);
        return 1;
    }
    free(bytecode);

    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        printf("Runtime Error: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }

    lua_close(L);
    return 0;
}