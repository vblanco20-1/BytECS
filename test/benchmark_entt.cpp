#include "BitTree.h"
#include <iostream>
#include <entt.hpp>

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.hpp"

TEST_CASE("join 50.000 benchmark", "[bit-tree,!benchmark]") {

	struct CA
	{
		float x, y, z;
	};
	struct CB
	{
		float r, g, b;
	};
	struct CC
	{
		float r, g, b;
	};
	struct CD
	{
		float r;
	};

	constexpr int num_entities = 50000;
	constexpr bool bRandomize = false;
	constexpr bool bBigRandomize = bRandomize && true;

	auto poolA = create_pool<CA>();
	auto poolB = create_pool<CB>();
	auto poolC = create_pool<CC>();
	auto poolD = create_pool<CD>();

	std::vector<entt::entity> all_entities;
	all_entities.reserve(num_entities);
	entt::registry test_reg;
	for (int i = 0; i <= num_entities; i += 1)
	{
		all_entities.push_back(test_reg.create());
	}

	auto rng = std::default_random_engine{};
	if (bRandomize) {
		std::shuffle(std::begin(all_entities), std::end(all_entities), rng);
	}


	int allcount = 0;
	auto start = std::chrono::system_clock::now();
	for (int i = 0; i <= num_entities; i += 1)
	{
		bool bhas = true;
		int itg = (to_integer(all_entities[i]) & 0xFFFFF);
		auto et = all_entities[i];
		if (itg % 2)
			//if(itg < 250000)
		{
			add_pool_element(&poolA, to_integer(et), CA{ i / 100000.f,0.f,0.f });
			test_reg.assign<CA>(all_entities[i], CA{ i / 100000.f,0.f,0.f });
		}
	}
	if (bBigRandomize) {
		std::shuffle(std::begin(all_entities), std::end(all_entities), rng);
	}
	for (int i = 0; i <= num_entities; i += 1)
	{
		bool bhas = true;
		int itg = (to_integer(all_entities[i]) & 0xFFFFF);
		auto et = all_entities[i];
		if (itg % 2 == 0)
		{
			add_pool_element(&poolB, to_integer(et), CB{ i / 100000.f,0.f,0.f });
			test_reg.assign<CB>(et, CB{ i / 100000.f,0.f,0.f });
		}
	}


	if (bBigRandomize) {
		std::shuffle(std::begin(all_entities), std::end(all_entities), rng);
	}
	for (int i = 0; i <= num_entities; i += 1)
	{
		bool bhas = true;
		int itg = (to_integer(all_entities[i]) & 0xFFFFF);
		auto et = all_entities[i];

		add_pool_element(&poolD, to_integer(et), CD{ i / 100000.f });
		test_reg.assign<CD>(et, CD{ i / 100000.f });
	}


	if (bBigRandomize) {
		std::shuffle(std::begin(all_entities), std::end(all_entities), rng);
	}
	for (int i = 0; i <= num_entities; i += 1)
	{
		bool bhas = true;
		int itg = (to_integer(all_entities[i]) & 0xFFFFF);
		auto et = all_entities[i];
		if (itg < num_entities / 2) {
			add_pool_element(&poolC, to_integer(et), CC{ i / 100000.f,0.f,0.f });
			test_reg.assign<CC>(et, CC{ i / 100000.f,0.f,0.f });
		}
	}

	BENCHMARK("join half: AB - BTree") {
		int iterations = 0;
		float maxval = 0;

		join_pools(&poolA, &poolB, [&iterations, &maxval](auto index, CA& a, CB& b) {
			maxval += a.x - b.r;
			iterations++;
			});

		return  maxval * iterations;
	};
	BENCHMARK("join half: AB - Entt") {
		int iterations = 0;
		float maxval = 0;

		test_reg.view<CA, CB >().each([&iterations, &maxval](entt::entity index, CA& a, CB& b)
			{
				maxval += a.x - b.r;
				iterations++;
			}
		);

		return  maxval * iterations;
	};

	BENCHMARK("join half: AC - BTree") {
		int iterations = 0;
		float maxval = 0;

		join_pools(&poolA, &poolC, [&iterations, &maxval](auto index, CA& a, CC& b) {
			maxval += a.x - b.r;
			iterations++;
			});

		return  maxval * iterations;
	};
	//test_reg.group<CA, CC >();
	BENCHMARK("join half: AC - Entt") {
		int iterations = 0;
		float maxval = 0;

		test_reg.view<CA, CC >().each([&iterations, &maxval](entt::entity index, CA& a, CC& b)
			{
				maxval += a.x - b.r;
				iterations++;
			}
		);

		return  maxval * iterations;
	};

	BENCHMARK("join half: AD - BTree") {
		int iterations = 0;
		float maxval = 0;

		join_pools(&poolA, &poolD, [&iterations, &maxval](auto index, CA& a, CD& b) {
			maxval += a.x - b.r;
			iterations++;
			});

		return  maxval * iterations;
	};
	BENCHMARK("join half: AD - Entt") {
		int iterations = 0;
		float maxval = 0;

		test_reg.view<CA, CD >().each([&iterations, &maxval](entt::entity index, CA& a, CD& b)
			{
				maxval += a.x - b.r;
				iterations++;
			}
		);

		return  maxval * iterations;
	};

	BENCHMARK("join half: DC - BTree") {
		int iterations = 0;
		float maxval = 0;

		join_pools(&poolC, &poolD, [&iterations, &maxval](auto index, CC& a, CD& b) {
			maxval += a.r - b.r;
			iterations++;
			});

		return  maxval * iterations;
	};
	BENCHMARK("join half: DC - Entt") {
		int iterations = 0;
		float maxval = 1;

		test_reg.view<CC, CD >().each([&iterations, &maxval](entt::entity index, CC& a, CD& b)
			{
				maxval += a.r - b.r;
				iterations++;
			}
		);

		return  maxval * iterations;
	};

	BENCHMARK("join half-noaccess: DC - BTree") {
		int iterations = 0;
		float maxval = 1;

		join_pools(&poolC, &poolD, [&iterations, &maxval](auto index, CC& a, CD& b) {
			//maxval += a.r - b.r;
			iterations++;
			});

		return  maxval * iterations;
	};
	BENCHMARK("join half-noaccess: DC - Entt") {
		int iterations = 0;
		float maxval = 1;

		test_reg.view<CC, CD >().each([&iterations, &maxval](auto index, CC& a, CD& b)
			//group.each([&niterations, &maxval](entt::entity index, CA& a, CB& b)
			{
				//maxval += a.r - b.r;
				iterations++;
			}
		);

		return  maxval * iterations;
	};

	BENCHMARK("join half-noaccess: AD - BTree") {
		int iterations = 0;
		float maxval = 1;

		join_pools(&poolA, &poolD, [&iterations, &maxval](auto index, CA& a, CD& b) {
			//maxval += a.x - b.r;
			iterations++;
			});

		return  maxval * iterations;
	};
	BENCHMARK("join half-noaccess: AD - Entt") {
		int iterations = 0;
		float maxval = 1;

		test_reg.view<CA, CD >().each([&iterations, &maxval](auto index, CA& a, CD& b)
			//group.each([&niterations, &maxval](entt::entity index, CA& a, CB& b)
			{
				//maxval += a.x - b.r;
				iterations++;
			}
		);

		return maxval * iterations;
	};


	BENCHMARK("join half: ADC - BTree") {
		int iterations = 0;
		float maxval = 0;

		join_pools(&poolA, &poolD, &poolC, [&iterations, &maxval](auto index, CA& a, CD& b, CC& c) {
			maxval += a.x - b.r * c.b;
			iterations++;
			});

		return maxval * iterations;
	};
	BENCHMARK("join half: ADC - Entt") {
		int iterations = 0;
		float maxval = 0;

		test_reg.view<CA, CD, CC >().each([&iterations, &maxval](auto index, CA& a, CD& b, CC& c) {
			maxval += a.x - b.r * c.b;
			iterations++;
			});


		return maxval * iterations;
	};
	BENCHMARK("join half: ABC - BTree") {
		int iterations = 0;
		float maxval = 0;

		join_pools(&poolA, &poolB, &poolC, [&iterations, &maxval](auto index, CA& a, CB& b, CC& c) {
			maxval += a.x - b.r * c.b;
			iterations++;
			});

		return maxval * iterations;
	};
	BENCHMARK("join half: ABC - Entt") {
		int iterations = 0;
		float maxval = 0;

		test_reg.view<CA, CB, CC >().each([&iterations, &maxval](auto index, CA& a, CB& b, CC& c) {
			maxval += a.x - b.r * c.b;
			iterations++;
			});


		return maxval * iterations;
	};


	BENCHMARK("join half - no access: ADC - BTree") {
		int iterations = 0;
		float maxval = 0;

		join_pools(&poolA, &poolD, &poolC, [&iterations, &maxval](auto index, CA& a, CD& b, CC& c) {
			//maxval += a.x - b.r * c.b;
			iterations++;
			});

		return maxval * iterations;
	};
	BENCHMARK("join half - no access: ADC - Entt") {
		int iterations = 0;
		float maxval = 0;

		test_reg.view<CA, CD, CC >().each([&iterations, &maxval](auto index, CA& a, CD& b, CC& c) {
			//maxval += a.x - b.r * c.b;
			iterations++;
			});


		return maxval * iterations;
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