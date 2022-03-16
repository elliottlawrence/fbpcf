/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>

#include "common/init/Init.h"

#include "fbpcf/engine/tuple_generator/ProductShareGeneratorFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotBasedBidirectionObliviousTransferFactory.h"
#include "fbpcf/engine/tuple_generator/oblivious_transfer/RcotHelper.h"
#include "fbpcf/engine/tuple_generator/test/TupleGeneratorTestHelper.h"
#include "fbpcf/engine/util/AesPrgFactory.h"
#include "fbpcf/engine/util/test/benchmarks/BenchmarkHelper.h"
#include "fbpcf/engine/util/test/benchmarks/NetworkedBenchmark.h"

namespace fbpcf::engine::tuple_generator {

class ProductShareGeneratorBenchmark : public util::NetworkedBenchmark {
 public:
  void setup() override {
    auto [agentFactory0, agentFactory1] = util::getSocketAgentFactories();
    agentFactory0_ = std::move(agentFactory0);
    agentFactory1_ = std::move(agentFactory1);

    senderFactory_ = std::make_unique<ProductShareGeneratorFactory<bool>>(
        std::make_unique<util::AesPrgFactory>(),
        std::make_unique<
            oblivious_transfer::RcotBasedBidirectionObliviousTransferFactory<
                bool>>(
            0, *agentFactory0_, oblivious_transfer::createFerretRcotFactory()));

    receiverFactory_ = std::make_unique<ProductShareGeneratorFactory<bool>>(
        std::make_unique<util::AesPrgFactory>(),
        std::make_unique<
            oblivious_transfer::RcotBasedBidirectionObliviousTransferFactory<
                bool>>(
            1, *agentFactory1_, oblivious_transfer::createFerretRcotFactory()));

    senderLeft_ = util::getRandomBoolVector(size_);
    senderRight_ = util::getRandomBoolVector(size_);

    receiverLeft_ = util::getRandomBoolVector(size_);
    receiverRight_ = util::getRandomBoolVector(size_);
  }

  void runSender() override {
    sender_ = senderFactory_->create(1);
    sender_->generateBooleanProductShares(senderLeft_, senderRight_);
  }

  void runReceiver() override {
    receiver_ = receiverFactory_->create(0);
    receiver_->generateBooleanProductShares(receiverLeft_, receiverRight_);
  }

  std::pair<uint64_t, uint64_t> getTrafficStatistics() override {
    return sender_->getTrafficStatistics();
  }

 private:
  size_t size_ = 1000000;

  std::unique_ptr<communication::IPartyCommunicationAgentFactory>
      agentFactory0_;
  std::unique_ptr<communication::IPartyCommunicationAgentFactory>
      agentFactory1_;

  std::unique_ptr<IProductShareGeneratorFactory> senderFactory_;
  std::unique_ptr<IProductShareGeneratorFactory> receiverFactory_;

  std::unique_ptr<IProductShareGenerator> sender_;
  std::unique_ptr<IProductShareGenerator> receiver_;

  std::vector<bool> senderLeft_;
  std::vector<bool> receiverLeft_;

  std::vector<bool> senderRight_;
  std::vector<bool> receiverRight_;
};

BENCHMARK_COUNTERS(ProductShareGenerator, counters) {
  ProductShareGeneratorBenchmark benchmark;
  benchmark.runBenchmark(counters);
}

} // namespace fbpcf::engine::tuple_generator

int main(int argc, char* argv[]) {
  facebook::initFacebook(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
