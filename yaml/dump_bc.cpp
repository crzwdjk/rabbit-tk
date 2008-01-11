#include "yaml.hpp"

int main(int argc, char ** argv)
{
	char * bc = yaml_fd_to_bytecode(0);
	printf("%s", bc);
}
