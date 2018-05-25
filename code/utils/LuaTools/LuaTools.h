#include <vector>
#include <map>
#include <string>

struct lua_State;
struct lua_Debug;

class LuaTools
{
public:
	LuaTools();
	~LuaTools();

	struct Smain {
		char **argv;
		int argc;
		int status;
	};

	static Smain smain;
	static lua_State *luaState;
	static const char *progname;

	static void StackDump(lua_State* l);

	static void print_usage(void);

	static int report(lua_State * L, int status);

	static int docall(lua_State * L, int narg, int clear);

	static void print_version(void);

	static void print_jit_status(lua_State * L);

	static void createargtable(lua_State * L, char ** argv, int argc, int argf);

	static int dofile(lua_State * L, const char * name);

	static int dostring(lua_State * L, const char * s, const char * name);

	static int dolibrary(lua_State * L, const char * name);

	static void write_prompt(lua_State * L, int firstline);

	static int incomplete(lua_State * L, int status);

	static int pushline(lua_State * L, int firstline);

	static int loadline(lua_State * L);

	static void dotty(lua_State * L);

	static int handle_script(lua_State * L, char ** argx);

	static int loadjitmodule(lua_State * L);

	static int runcmdopt(lua_State * L, const char * opt);

	static int dojitcmd(lua_State * L, const char * cmd);

	static int dojitopt(lua_State * L, const char * opt);

	static int dobytecode(lua_State * L, char ** argv);

	static int collectargs(char ** argv, int * flags);

	static int runargs(lua_State * L, char ** argv, int argn);

	static int handle_luainit(lua_State * L);

	static int pmain(lua_State * L);

	int lmain(int argc, char ** argv);

private:
	static void laction(int i);
	static void lstop(lua_State * L, lua_Debug * ar);
	static void l_message(const char * pname, const char * msg);
	static int traceback(lua_State * L);
};