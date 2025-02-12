#include <bitset>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

#include "common/alexnet.h"
#include "protocols/share_wrapper.h"
#include "secure_type/secure_signed_integer.h"
#include "secure_type/secure_unsigned_integer.h"
#include "utility/config.h"

#include "emp-aby/converter/a2bconverter.h"
#include "emp-aby/converter/b2aconverter.h"
#include "emp-aby/io/multi-io.hpp"

#include <cmath>
#include <random>
#include <regex>

#include <fmt/format.h>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <stdexcept>

#include "base/party.h"
#include "communication/communication_layer.h"
#include "communication/tcp_transport.h"

// abbreviate namespace
namespace program_options = boost::program_options;
namespace mo = encrypto::motion;
using namespace emp;

bool CheckPartyArgumentSyntax(const std::string& party_argument);

std::pair<program_options::variables_map, bool> ParseProgramOptions(int ac, char* av[]);

encrypto::motion::PartyPointer CreateParty(const program_options::variables_map& user_options);

static int party_id;
static int num_party;
static int threads;
static int port;
static double lut_total_com;
static double total_time;

std::vector<mo::ShareWrapper> make_share_bool(mo::PartyPointer& party, bool* b, const uint32_t num,
                                              const int l) {
  const std::size_t num_party{party->GetConfiguration()->GetNumOfParties()};

  std::vector<mo::ShareWrapper> input_values(num_party);
  std::vector<mo::BitVector<>> bitVectors;

  for (size_t i = 0; i < l; ++i) {
    mo::BitVector<> bitVector(0);  // Initialize BitVector with the correct size
    for (size_t j = 0; j < num; ++j) bitVector.Append(b[j * l + i]);

    bitVectors.push_back(std::move(bitVector));  // Add the BitVector to the list
  }
  for (std::size_t i = 0; i < num_party; ++i) {
    input_values[i] = party->In<mo::MpcProtocol::kBooleanGmw>(bitVectors, i);
  }

  return input_values;
}

// 将一维数组转换为三维数组
std::vector<std::vector<std::vector<int64_t>>> ConvertTo3D(int64_t* input, int channels, int height, int width) {
    std::vector<std::vector<std::vector<int64_t>>> tensor(channels, std::vector<std::vector<int64_t>>(height, std::vector<int64_t>(width)));
    int idx = 0;
    for(int c = 0; c < channels; ++c){
        for(int h = 0; h < height; ++h){
            for(int w = 0; w < width; ++w){
                tensor[c][h][w] = input[idx++];
            }
        }
    }
    return tensor;
}

// 将三维数组转换回一维数组
int64_t* ConvertTo1D(std::vector<std::vector<std::vector<int64_t>>> tensor) {
    int channels = tensor.size();
    int height = tensor[0].size();
    int width = tensor[0][0].size();
    int64_t* output = new int64_t[channels * height * width];
    int idx = 0;
    for(int c = 0; c < channels; ++c){
        for(int h = 0; h < height; ++h){
            for(int w = 0; w < width; ++w){
                output[idx++] = tensor[c][h][w];
            }
        }
    }
    return output;
}


// ------------------ ReLU 封装函数 ------------------
int64_t* ReLU(int64_t* input, mo::PartyPointer& party, HE<MultiIOBase>* he, uint32_t num, int l, int module_p,
              ThreadPool* pool, MPIOChannel<MultiIOBase>* io, 
              A2BConverter<MultiIOBase>* a2b_converter, B2AConverter<MultiIOBase>* b2a_converter) {
  // 模数参数
  mo::SecureUnsignedInteger mod_p_half =
      party->In<mo::MpcProtocol::kBooleanGmw>(mo::ToInput((module_p / 2) + 1), 0);
  mo::ShareWrapper p = party->In<mo::MpcProtocol::kBooleanGmw>(mo::ToInput(module_p), 0);
  mo::ShareWrapper zero = party->In<mo::MpcProtocol::kBooleanGmw>(mo::ToInput(0), 0);

  const long long int q = he->q;
  bool* b = new bool[num * l];
  // std::cout << "Start A2B" << std::endl;
  a2b_converter->convert(b, input, num);

  std::vector<mo::ShareWrapper> input_values = make_share_bool(party, b, num, l);

  std::vector<std::vector<mo::ShareWrapper>> val(num_party);
  for (std::size_t i = 0; i < num_party; ++i) {
    val[i] = input_values[i].Unsimdify();
  }

  std::vector<mo::ShareWrapper> result(val[0].size());

  for (std::size_t i = 0; i < val[0].size(); ++i) {
    // Reconstruct the value and perform comparison
    mo::ShareWrapper sum = val[0][i];
    for (std::size_t j = 1; j < num_party; ++j) {
      sum = sum ^ val[j][i];
    }

    // 2. Get the value's true signature
    auto comparison_result = mo::SecureUnsignedInteger{sum} > mod_p_half;

    // 3. ReLU
    sum = comparison_result.Mux(zero.Get(), sum.Get());
  
    // 4. Truncate and map to Field
    sum = sum.RightShiftWithFill(5);

    result[i] = sum;
  }
  auto results = mo::ShareWrapper::Simdify(result);

  // 6. FB2A
  int length = l * num;
  int64_t* arithmetic = new int64_t[num];
  auto bit_vector_vec = results.As<std::vector<mo::BitVector<>>>();

  // 按行优先顺序将每个比特存储到一维布尔数组中
  for (std::size_t i = 0; i < l; ++i) {
    std::string bitString = bit_vector_vec[i].AsString();  // 将 BitVector 转换为字符串
    for (std::size_t j = 0; j < num; ++j) {
      b[j * l + i] = (bitString[j] == '1');
    }
  }
  
  // std::cout << "Start B2A" << std::endl;
  b2a_converter->convert(arithmetic, b, length, l);

  for (size_t i = 0; i < num; ++i) arithmetic[i] = arithmetic[i] % q;

  delete b;
  delete input;
  return arithmetic;
}

// ------------------ 卷积层模拟 ------------------
int64_t* ConvLayer(int64_t* input) {
  // 卷积层的模拟：真实代码中需包含卷积计算过程
  // 为了简化，在此仅返回输入作为输出
  return input;
}

// ------------------ 池化层模拟 ------------------
int64_t* PoolLayer(int rows, int cols, int64_t *input, int module_p, mo::PartyPointer& party, HE<MultiIOBase>* he, 
          uint32_t num, int l, ThreadPool* pool, MPIOChannel<MultiIOBase>* io, 
          A2BConverter<MultiIOBase>* a2b_converter, B2AConverter<MultiIOBase>* b2a_converter) {
    int64_t *max_i = new int64_t[rows];   // Max value in each row
    int64_t *compare_with = new int64_t[rows];

    for (int r = 0; r < rows; r++) {
      max_i[r] = input[r * cols];
    }
    for (int c = 1; c < cols; c++) {
      for (int r = 0; r < rows; r++) {
        compare_with[r] = max_i[r] - input[r * cols + c];
      }
      compare_with = ReLU(compare_with, party, he, rows, l, module_p, pool, io, a2b_converter, b2a_converter);
      for (int r = 0; r < rows; r++) {
        max_i[r] = (max_i[r] + input[r * cols + c]) % module_p;
      }
    }
    
  return max_i;
}

// ------------------ Flatten 层 ------------------
int64_t* Flatten(int64_t* input) {
  // 将输入多维张量展平为一维
  return input;  // Placeholder，只是返回输入，需根据实际情况调整
}

// ------------------ 全连接层模拟 ------------------
int64_t* FullyConnectedLayer(int64_t* input) {
  // 全连接层的模拟
  return input;
}

// ------------------ 输出层 ------------------
int64_t* OutputLayer(int64_t* input) {
  // 输出层的模拟
  return input;
}

// ------------------ AlexNet 前向传播 ------------------
int64_t* ForwardAlexNet(mo::PartyPointer& party, int64_t* input, uint32_t num, int l, int module_p, ThreadPool* pool,
                        HE<MultiIOBase>* he, MPIOChannel<MultiIOBase>* io) {
  // AlexNet 的各层结构：
  // Conv1 -> ReLU -> Pool -> Conv2 -> ReLU -> Pool -> Flatten -> FC1 -> ReLU -> FC2 -> Output

  // Setup phase
  int pool_size = std::max(static_cast<int>(20 / num_party - 1), 0) * num_party + num_party;
  if (num_party > 16) pool_size = 16;
  std::cout << "pool size " << pool_size << std::endl;

  int channels = 13;
  int height = 13;
  int width = 256;

  MPSIMDCircExec<MultiIOBase>* simd_circ =
      new MPSIMDCircExec<MultiIOBase>(num_party, party_id + 1, pool, io);

  A2BConverter<MultiIOBase>* a2b_converter =
      new A2BConverter<MultiIOBase>(num_party, party_id + 1, io, pool, he, simd_circ, pool_size);

  B2AConverter<MultiIOBase>* b2a_converter =
      new B2AConverter<MultiIOBase>(num_party, party_id + 1, io, pool, he, pool_size);

  lut_total_com = io->get_total_bytes_sent();
  // Conv1
  std::cout << "Start Conv1" << std::endl;
  auto conv1 = ConvLayer(input);

  // // ReLU1
  // std::cout << "Start ReLU1" << std::endl;
  // auto start = clock_start();
  // auto relu1 = ReLU(conv1, party, he, num, l, module_p, pool, io, a2b_converter, b2a_converter);
  // auto timeused = time_from(start);
  // std::cout << party_id << "\tReLU1 costs\t" << timeused / (1000 * num) << " ms\t per data" << std::endl;

  // Pool1
  std::cout << "Start Pool1" << std::endl;
  // auto pool1 = conv1;
  auto start = clock_start();
  auto pool1 = PoolLayer(10, 10, conv1, module_p, party, he, num, l, pool, io, a2b_converter, b2a_converter);
  auto timeused = time_from(start);
  total_time = timeused;
  std::cout << party_id << "\tPool1 costs\t" << timeused / (1000 * num) << " ms\t per data" << std::endl;

  // // Conv2
  // std::cout << "Start Conv2" << std::endl;
  // auto conv2 = ConvLayer(pool1);

  // // ReLU2
  // std::cout << "Start ReLU2" << std::endl;
  // start = clock_start();
  // auto relu2 = ReLU(conv2, party, he, num, l, module_p, pool, io, a2b_converter, b2a_converter);
  // timeused = time_from(start);
  // std::cout << party_id << "\tReLU2\t" << timeused / 1000 << " ms\t" << std::endl;

  // // Pool2
  // auto pool2 = relu2;
  // // auto pool2 = PoolLayer(rows2, cols2, relu2, module_p, party, he, num, l, pool, io, a2b_converter, b2a_converter);

  // // Flatten
  // std::cout << "Start Flatten" << std::endl;
  // auto flatten = Flatten(pool2);

  // // Fully Connected Layer 1 (FC1)
  // std::cout << "Start FC1" << std::endl;
  // auto fc1 = FullyConnectedLayer(flatten);

  // // ReLU3
  // std::cout << "Start ReLU3" << std::endl;
  // auto relu3 = ReLU(fc1, party, he, num, l, module_p, pool, io, a2b_converter, b2a_converter);

  // // Fully Connected Layer 2 (FC2)
  // std::cout << "Start FC2" << std::endl;
  // auto fc2 = FullyConnectedLayer(relu3);

  // Output Layer
  std::cout << "Start Output" << std::endl;
  auto output = OutputLayer(pool1);

  return output;  // 返回最终的输出
}

// ------------------ AlexNet 输入处理 ------------------
int64_t* GetInputsForAlexNet(mo::PartyPointer& party, HE<MultiIOBase>* he, PRG* prg, uint32_t num,
                             int l) {
  // 创建输入（这里以随机数据代替实际图像数据）
  const long long int q = he->q;
  int64_t* a = new int64_t[num];

  std::cout << "num: " << num << std::endl;
  std::cout << "l (bit length): " << l << std::endl;

  prg->random_data(a, num * sizeof(int64_t));
  for (int i = 0; i < num; ++i) {
    a[i] = ((a[i] % he->q) + he->q) % he->q;
  }

  // 示例输入
  return a;
}

// ------------------ 结果处理 ------------------
void ProcessOutputs(int64_t* output, int num) {
  // 处理并输出结果
  // for (int i = 0; i < num; i++) {
  //   std::cout << "Final output: " << output[i].As<std::int64_t>() << std::endl;
  // }
  return;
}

// ------------------ 主函数 ------------------
int main(int ac, char* av[]) {
  auto [user_options, help_flag] = ParseProgramOptions(ac, av);
  // if help flag is set - print allowed command line arguments and exit
  if (help_flag) return EXIT_SUCCESS;

  // 初始化隐私计算协议的 party
  auto party = CreateParty(user_options);
  uint32_t num = user_options["input"].as<std::uint32_t>();

  num_party = party->GetConfiguration()->GetNumOfParties();
  // std::cout << std::to_string(num_party) << std::endl;
  party_id = party->GetConfiguration()->GetMyId();
  // std::cout << "My party_id is " << std::to_string(party_id) << std::endl;
  threads = 4;
  // ot_total_com = 0;
  // 
  std::cout << "threads = " << threads << std::endl;
  
  port = 12345;
  const uint64_t module_p = 3221225473;

  // (pre-)allocate input values
  std::vector<mo::ShareWrapper> input_values(num_party);

  std::vector<std::pair<std::string, unsigned short>> net_config;

  for (int i = 0; i < num_party; ++i) {
    std::string s = "127.0.0.1";
    uint port_i = (port + 4 * num_party * i);
    net_config.push_back(std::make_pair(s, port_i));
  }

  ThreadPool pool(threads);

  MultiIO* io = new MultiIO(party_id + 1, num_party, net_config);
  io->setup_ot_ios();
  std::cout << "io setup" << std::endl;

  const long long int modulus = (1L << 32) - (1L << 30) + 1;
  HE<MultiIOBase>* he = new HE<MultiIOBase>(num_party, io, &pool, party_id + 1, modulus);
  he->multiplication_keygen();
  he->rotation_keygen();
  const long long int q = he->q;
  PRG prg;
  // int num = 2000;
  const int l = ceil(log2(q));

  std::cout << "p = " << he->cc->GetCryptoParameters()->GetPlaintextModulus() << std::endl;
  std::cout << "n = " << he->cc->GetCryptoParameters()->GetElementParams()->GetCyclotomicOrder() / 2
            << std::endl;

  // 获取输入
  int64_t* inputs = GetInputsForAlexNet(party, he, &prg, num, l);

  // 前向传播
  int64_t* final_output = ForwardAlexNet(party, inputs, num, l, module_p, &pool, he, io);

  lut_total_com = io->get_total_bytes_sent() - lut_total_com;
  // 处理输出
  ProcessOutputs(final_output, num);

  // 结束协议
  std::cout << "Run protocol" << std::endl;
  auto start = clock_start();

  party->Run();
  party->Finish();

  double timeused = time_from(start);
  total_time += timeused;
  std::cout << party_id << "\tMOTION Protocol costs \t" << timeused / (1000 * num) << " ms\t per data" << std::endl;

  std::cout << party_id << "\tProtocol total costs \t" << total_time / (1000 * num) << " ms\t per data" << std::endl;
  std::cout << party_id << "\tProtocol total costs \t" << total_time / 1000 << " ms\t" << std::endl;

  std::cout << "MOTION OT communication: " << ot_total_com << " bytes" << std::endl;
  std::cout << "LUT OT communication: " << lut_total_com << " bytes" << std::endl;
  std::cout << "Total OT communication: " << lut_total_com + ot_total_com << " bytes" << std::endl;

  delete io;
  delete he;
  return 0;
}

const std::regex kPartyArgumentRegex(
    "(\\d+),(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}),(\\d{1,5})");

bool CheckPartyArgumentSyntax(const std::string& party_argument) {
  // other party's id, IP address, and port
  return std::regex_match(party_argument, kPartyArgumentRegex);
}

std::tuple<std::size_t, std::string, std::uint16_t> ParsePartyArgument(
    const std::string& party_argument) {
  std::smatch match;
  std::regex_match(party_argument, match, kPartyArgumentRegex);
  auto id = boost::lexical_cast<std::size_t>(match[1]);
  auto host = match[2];
  auto port = boost::lexical_cast<std::uint16_t>(match[3]);
  return {id, host, port};
}

// <variables map, help flag>
std::pair<program_options::variables_map, bool> ParseProgramOptions(int ac, char* av[]) {
  using namespace std::string_view_literals;
  constexpr std::string_view kConfigFileMessage =
      "configuration file, other arguments will overwrite the parameters read from the configuration file"sv;
  bool print, help;
  boost::program_options::options_description description("Allowed options");
  // clang-format off
  description.add_options()
      ("help,h", program_options::bool_switch(&help)->default_value(false),"produce help message")
      ("disable-logging,l","disable logging to file")
      ("input,i", program_options::value<uint32_t>(), "This party's input")
      ("print-configuration,p", program_options::bool_switch(&print)->default_value(false), "print configuration")
      ("configuration-file,f", program_options::value<std::string>(), kConfigFileMessage.data())
      ("my-id", program_options::value<std::size_t>(), "my party id")
      ("parties", program_options::value<std::vector<std::string>>()->multitoken(), "info (id,IP,port) for each party e.g., --parties 0,127.0.0.1,23000 1,127.0.0.1,23001");
  // clang-format on

  program_options::variables_map user_options;

  program_options::store(program_options::parse_command_line(ac, av, description), user_options);
  program_options::notify(user_options);

  // argument help or no arguments (at least a configuration file is expected)
  if (user_options["help"].as<bool>() || ac == 1) {
    std::cout << description << "\n";
    return std::make_pair<program_options::variables_map, bool>({}, true);
  }

  // read configuration file
  if (user_options.count("configuration-file")) {
    std::ifstream ifs(user_options["configuration-file"].as<std::string>().c_str());
    program_options::variables_map user_option_config_file;
    program_options::store(program_options::parse_config_file(ifs, description), user_options);
    program_options::notify(user_options);
  }

  // print parsed parameters
  if (user_options.count("my-id")) {
    if (print) std::cout << "My id " << user_options["my-id"].as<std::size_t>() << std::endl;
  } else
    throw std::runtime_error("My id is not set but required");

  if (user_options.count("parties")) {
    const std::vector<std::string> other_parties{
        user_options["parties"].as<std::vector<std::string>>()};
    std::string parties("Other parties: ");
    for (auto& p : other_parties) {
      if (CheckPartyArgumentSyntax(p)) {
        if (print) parties.append(" " + p);
      } else {
        throw std::runtime_error("Incorrect party argument syntax " + p);
      }
    }
    if (print) std::cout << parties << std::endl;
  } else
    throw std::runtime_error("Other parties' information is not set but required");

  return std::make_pair(user_options, help);
}

encrypto::motion::PartyPointer CreateParty(const program_options::variables_map& user_options) {
  const auto parties_string{user_options["parties"].as<const std::vector<std::string>>()};
  const auto number_of_parties{parties_string.size()};
  const auto my_id{user_options["my-id"].as<std::size_t>()};
  if (my_id >= number_of_parties) {
    throw std::runtime_error(fmt::format(
        "My id needs to be in the range [0, #parties - 1], current my id is {} and #parties is {}",
        my_id, number_of_parties));
  }

  encrypto::motion::communication::TcpPartiesConfiguration parties_configuration(number_of_parties);

  for (const auto& party_string : parties_string) {
    const auto [party_id, host, port] = ParsePartyArgument(party_string);
    if (party_id >= number_of_parties) {
      throw std::runtime_error(
          fmt::format("Party's id needs to be in the range [0, #parties - 1], current id "
                      "is {} and #parties is {}",
                      party_id, number_of_parties));
    }
    parties_configuration.at(party_id) = std::make_pair(host, port);
  }
  encrypto::motion::communication::TcpSetupHelper helper(my_id, parties_configuration);
  auto communication_layer = std::make_unique<encrypto::motion::communication::CommunicationLayer>(
      my_id, helper.SetupConnections());
  auto party = std::make_unique<encrypto::motion::Party>(std::move(communication_layer));
  auto configuration = party->GetConfiguration();
  // disable logging if the corresponding flag was set
  const auto logging{!user_options.count("disable-logging")};
  configuration->SetLoggingEnabled(logging);
  return party;
}
