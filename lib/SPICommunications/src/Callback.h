#ifndef CALLBACK_H_
#define CALLBACK_H_

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

template<class ... Arguments>
class Callback
{
  using Function = std::function<void(Arguments ...)>;
  using FunctionPtr = std::shared_ptr<Function>;
  using Functions = std::vector<FunctionPtr>;

 public:
  FunctionPtr addListener(const Function f) {
    FunctionPtr fp = std::make_shared<Function>(f);
    _functions.push_back(fp);
    return fp;
  }

  void removeListener(const FunctionPtr &fp){
    for (auto it = _functions.begin(); it != _functions.end();)
    {
      if (*it == fp) {
        _functions.erase(it);
        return ;
      } else
        ++it;
    }
  }

  void operator()(const Arguments &... args) const{
    for (const auto & f : _functions)
      f.get()->operator()( args ... );
  }

 private:
  Functions _functions;
};


#endif  //NEURONWIRED_LIB_SPICOMMUNICATIONS_SRC_CALLBACK_H_
