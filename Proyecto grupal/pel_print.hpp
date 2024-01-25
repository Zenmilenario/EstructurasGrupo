#ifndef PEL_PRINT_HPP_UEM_LIBRARY
#define PEL_PRINT_HPP_UEM_LIBRARY

/*
   pel_print.hpp
   Header file containing pel::print and pel::println functions to format 'args'
   according to the format string 'message', and print the result to stdout
   -----------------------------------------------------------------------------
   Programación con Estructuras Lineales
   Grado en Ingeniería Informática Online | Universidad Europea de Madrid
   -----------------------------------------------------------------------------
   MIT License

   Copyright (c) 2023 Daniel Gómez Vergel - daniel.gomez[at]universidadeuropea.es
      
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include <format>
#include <iostream>

namespace pel {

   template<typename... Args>
   void print(std::format_string<Args...> const& message, Args&&... args)
   {
      std::printf(std::vformat(message.get(), std::make_format_args(args...)).c_str());
   }

   template<typename... Args>
   void println(std::format_string<Args...> const& message, Args&&... args)
   {
      std::printf(std::vformat(message.get(), std::make_format_args(args...)).c_str());
      std::putc('\n', stdout);
   }

} // namespace pel

#endif