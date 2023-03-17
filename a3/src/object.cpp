// object.cpp

#include "object.h"
#include "linalg.h"


// input operator

std::ostream& operator << ( std::ostream& stream, State const & state )

{
  stream << state.x << endl
	 << state.q << endl
	 << state.v << endl
	 << state.w << endl;
  
  return stream;
}


// output operator

std::istream& operator >> ( std::istream& stream, State & state )

{
  stream >> state.x >> state.q >> state.v >> state.w;

  return stream;
}

