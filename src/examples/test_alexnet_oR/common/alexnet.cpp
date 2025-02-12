#include "alexnet.h"

#include "algorithm/algorithm_description.h"
#include "protocols/share_wrapper.h"
#include "utility/config.h"

void EvaluateProtocol(encrypto::motion::PartyPointer& party) {
  // put your code here
  party->Run();
  party->Finish();
}