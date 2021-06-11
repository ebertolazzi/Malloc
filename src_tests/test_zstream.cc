/*--------------------------------------------------------------------------*\
 |                                                                          |
 |  Copyright (C) 2017                                                      |
 |                                                                          |
 |         , __                 , __                                        |
 |        /|/  \               /|/  \                                       |
 |         | __/ _   ,_         | __/ _   ,_                                | 
 |         |   \|/  /  |  |   | |   \|/  /  |  |   |                        |
 |         |(__/|__/   |_/ \_/|/|(__/|__/   |_/ \_/|/                       |
 |                           /|                   /|                        |
 |                           \|                   \|                        |
 |                                                                          |
 |      Enrico Bertolazzi                                                   |
 |      Dipartimento di Ingegneria Industriale                              |
 |      Universita` degli Studi di Trento                                   |
 |      email: enrico.bertolazzi@unitn.it                                   |
 |                                                                          |
\*--------------------------------------------------------------------------*/

#include "Utils.hh"
#include "Utils/zstream/ozstream.hpp"

#include <fstream>

int
main() {
  try {
    std::ofstream file("test.txt.gz");;
    zstream::ogzstream gzfile(file); 
    gzfile << "pippo\n";
    gzfile << "pluto\n";
    gzfile << "paperino\n";
    gzfile << "paperone\n";
    gzfile << "nonna papera\n";
    gzfile.close();
    file.close();
  } catch ( std::exception const & exc ) {
    std::cout << "Error: " << exc.what() << '\n';
  } catch ( ... ) {
    std::cout << "Unknown error\n";
  }
  std::cout << "All done folks\n\n";
  return 0;
}
