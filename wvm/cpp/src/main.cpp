#include <optional>
#include <stdlib.h>
#include <signal.h>

#include "lib/ansi.h"
#include "mode/ServerMode.h"
#include "net/Server.h"

std::optional<WVM::Mode::ServerMode> server;

void usage() {
	std::cerr << "Usage:\n- wvm server <executable>\n- wvm registers\n- wvm memory\n- wvm console\n";
}

int main(int argc, char **argv) {
	if (argc < 2) {
		usage();
		return 1;
	}

	std::string arg = argv[1];

	if (arg == "server") {
		server.emplace(44902);
		server->run();
		signal(SIGINT, +[](int) { server->stop(); });
	} else if (arg == "registers") {
		
	} else if (arg == "memory") {

	} else if (arg == "console") {

	} else {
		usage();
		return 1;
	}
}
