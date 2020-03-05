#include <iostream>
#include <vector>

#include <tdc/random/vector.hpp>
#include <tdc/stat/phase.hpp>
#include <tdc/vec/int_vector.hpp>

#include <tlx/cmdline_parser.hpp>

using namespace tdc;

struct {
    size_t num = 1'000'000ULL;
    std::vector<uint64_t> data;
    
    size_t num_queries = 10'000'000ULL;
    std::vector<size_t> queries;

    uint64_t seed = random::DEFAULT_SEED;
} options;

stat::Phase benchmark_phase(std::string&& title) {
    stat::Phase phase(std::move(title));
    phase.log("num", options.num);
    phase.log("queries", options.num_queries);
    phase.log("seed", options.seed);
    return phase;
}

template<typename C>
void bench(C constructor) {
    auto iv = constructor(options.num);
    
    stat::Phase::wrap("set_seq", [&iv](){
        for(size_t i = 0; i < options.num; i++) {
            iv[i] = options.data[i];
        }
    });
    stat::Phase::wrap("get_seq", [&iv](stat::Phase& phase){
        uint64_t chk = 0;
        for(size_t i = 0; i < options.num; i++) {
            chk += iv[i];
        }
        phase.log("chk", chk);
    });
    stat::Phase::wrap("get_rnd", [&iv](stat::Phase& phase){
        uint64_t chk = 0;
        for(size_t j = 0; j < options.num_queries; j++) {
            const size_t i = options.queries[j];
            chk += iv[i];
        }
        phase.log("chk", chk);
    });
    stat::Phase::wrap("set_rnd", [&iv](){
        for(size_t j = 0; j < options.num_queries; j++) {
            const size_t i = options.queries[j];
            iv[i] = i + j;
        }
    });
}

int main(int argc, char** argv) {
    tlx::CmdlineParser cp;
    cp.add_bytes('n', "num", options.num, "The size of the bit vetor (default: 1M).");
    cp.add_bytes('q', "queries", options.num_queries, "The size of the bit vetor (default: 10M).");
    cp.add_bytes('s', "seed", options.seed, "The random seed.");
    if(!cp.process(argc, argv)) {
        return -1;
    }

    // generate numbers
    options.data = random::vector<uint64_t>(options.num, UINT64_MAX, options.seed);

    // generate queries
    options.queries = random::vector<size_t>(options.num_queries, options.num - 1, options.seed);
    
    // std::vector<uint8>
    {
        auto result = benchmark_phase("std(8)");
        bench([](const size_t sz){ return std::vector<uint8_t>(sz); });
        
        result.suppress([&](){
            std::cout << "RESULT algo=std(8) " << result.to_keyval() << " " << result.subphases_keyval() << " " << result.subphases_keyval("chk") << std::endl;
        });
    }

    // std::vector<uint16>
    {
        auto result = benchmark_phase("std(16)");
        bench([](const size_t sz){ return std::vector<uint16_t>(sz); });
        
        result.suppress([&](){
            std::cout << "RESULT algo=std(16) " << result.to_keyval() << " " << result.subphases_keyval() << " " << result.subphases_keyval("chk") << std::endl;
        });
    }

    // std::vector<uint32>
    {
        auto result = benchmark_phase("std(32)");
        bench([](const size_t sz){ return std::vector<uint32_t>(sz); });
        
        result.suppress([&](){
            std::cout << "RESULT algo=std(32) " << result.to_keyval() << " " << result.subphases_keyval() << " " << result.subphases_keyval("chk") << std::endl;
        });
    }
    
    // std::vector<uint64>
    {
        auto result = benchmark_phase("std(64)");
        bench([](const size_t sz){ return std::vector<uint64_t>(sz); });
        
        result.suppress([&](){
            std::cout << "RESULT algo=std(64) " << result.to_keyval() << " " << result.subphases_keyval() << " " << result.subphases_keyval("chk") << std::endl;
        });
    }
    
    // tdc::vec::IntVector
    for(size_t w = 2; w < 64; w++)
    {
        auto result = benchmark_phase("tdc");
        bench([w](const size_t sz){ return vec::IntVector(sz, w); });
        
        result.suppress([&](){
            std::cout << "RESULT algo=tdc(" << w << ") " << result.to_keyval() << " " << result.subphases_keyval() << " " << result.subphases_keyval("chk") << std::endl;
        });
    }
    
    return 0;
}
