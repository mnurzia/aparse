import pack_v4

class Project(pack_v4.Project):
    def generate(self):
        self.prefix = "aparse"
        self.version = "0.0.1"
        self.bits = ["exports", "unused", "hook_malloc"]
        self.apis = ["aparse_api.h"]
        self.headers = ["aparse_internal.h"]
        self.sources = [
            "aparse_error.c",
            "aparse_opt_bool.c",
            "aparse_opt_int.c",
            "aparse_opt.c",
            "aparse_parse.c",
            "aparse_state.c",
            "aparse_test.c"
        ]
        self.cstd = "c89"
        self.config = {
            "USE_MALLOC": {
                "type": str,
                "help": "Whether or not to use malloc.",
                "default": "1"
            }
        }
