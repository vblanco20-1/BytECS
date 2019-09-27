#include "BitTree.h"
#include <iostream>
#include <entt.hpp>

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.hpp"

TEST_CASE("create benchmark", "[bit-tree,!benchmark]") {

	struct CA
	{
		float x, y, z;
	};
	struct CB
	{
		float x, y;
	};
	struct CC
	{
	};

	BENCHMARK("create 10 BTree: contiguous") {
		node_pool_index = 0;
		auto poolA = create_pool<CA>();

		for (int i = 0; i < 10; i++) {
			add_pool_element(&poolA,i, CA{ i / 100000.f,0.f,0.f });
		}
		return poolA.Dense.size();
	};
	BENCHMARK("create 100 BTree: contiguous") {
		node_pool_index = 0;
		auto poolA = create_pool<CA>();

		for (int i = 0; i < 100; i++) {
			add_pool_element(&poolA, i, CA{ i / 100000.f,0.f,0.f });
		}
		return poolA.Dense.size();
	};
	BENCHMARK("create 10000 BTree: contiguous") {

		node_pool_index = 0;
		auto poolA = create_pool<CA>();
		for (int i = 0; i < 10000; i++) {
			add_pool_element(&poolA,i, CA{ i / 100000.f,0.f,0.f });
		}
		return poolA.Dense.size();
	};



	BENCHMARK("create 1.000.000 BTree, 3 pools: contiguous") {

		node_pool_index = 0;
		auto poolA = create_pool<CA>();
		auto poolB = create_pool<CB>();
		auto poolC = create_pool<CC>();

		poolA.Dense.reserve(1000000);
		poolB.Dense.reserve(1000000);
		poolC.Dense.reserve(1000000);

		poolA.Reverse.reserve(1000000);
		poolB.Reverse.reserve(1000000);
		poolC.Reverse.reserve(1000000);

		for (int i = 0; i < 1000000; i++) {
			add_pool_element(&poolA, i, CA{ i / 100000.f,0.f,0.f });
			add_pool_element(&poolB, i, CB{ i / 100000.f,0.f });
			add_pool_element(&poolC, i, CC{});
		}
		return poolA.Dense.size();
	};
}

int main(int argc, char* argv[])
{
	Catch::Session session; // There must be exactly one instance

	int returnCode = session.applyCommandLine(argc, argv);
	if (returnCode != 0) // Indicates a command line error
		return returnCode;

	int numFailed = session.run();

	return numFailed;
}