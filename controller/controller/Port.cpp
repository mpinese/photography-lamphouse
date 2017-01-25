//!
//! @file 				Port.cpp
//! @author 			Geoffrey Hunter <gbmhunter@gmail.com> (www.mbedded.ninja)
//! @edited 			n/a
//! @created			2013-06-17
//! @last-modified		2014-09-15
//! @brief 				Port specific functions.
//! @details
//!		See README.rst in root dir for more info.

#ifndef __cplusplus
	#error Please build with C++ compiler
#endif

//===============================================================================================//
//========================================= INCLUDES ============================================//
//===============================================================================================//

// Associated header file
#include "Port.hpp"
#include "WProgram.h"

//===============================================================================================//
//======================================== NAMESPACE ============================================//
//===============================================================================================//

namespace Fp 
{
	void Port::DebugPrint(char* msg)
	{
		Serial.print(msg);
	}

} // namespace Fp

// EOF
