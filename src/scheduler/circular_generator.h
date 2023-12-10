#ifndef CIRCULAR_GENERATOR_H
#define CIRCULAR_GENERATOR_H

#include <cassert>

namespace saber {
class Circular_Generator_Iterator {
private:
  static const int NULL_POS = -1;
  int _x;
  const int _start, _modular;
  const bool _batching;
public:
  Circular_Generator_Iterator(int start, int modular, bool batching) : _modular(modular), _start(start), _batching(batching) {
	  _x = _batching?0:start;
    if (modular <= 0 || start < 0 || start >= modular) {
      _x = NULL_POS;
    }
  }

  int operator*() { return _x; }
  bool operator!=(const Circular_Generator_Iterator &other) {
    return _x != other._x;
  }
  Circular_Generator_Iterator& operator++() {
	  if (_batching){
			++_x;
			if (_x > _start) {_x = -1;}
	  }
	  else{
			_x = (_x + 1) % _modular;
			if (_x == _start) { _x = -1; } //stops iteration when returns to start point
	  }
    return *this;
  }
};
// A circular generator that starts at "start" and returns to 0 if x increases to modular.
// Stops iteration (evaluate as false) if returns to start.
class Circular_Generator {
private:
        const int _modular, _start_pos;
        const bool _batching;
public:
        Circular_Generator(int start, int mod, bool batching) :_modular(mod), _start_pos(start), _batching(batching){
                assert(mod > 0 && start >= 0 && start < mod);
        }
        Circular_Generator_Iterator begin() {
                return Circular_Generator_Iterator{_start_pos, _modular, _batching};
        }
        Circular_Generator_Iterator end() {
                return Circular_Generator_Iterator{_modular,_modular, _batching};
        }
};


}  //namespace saber
#endif //CIRCULAR_GENERATOR_H
