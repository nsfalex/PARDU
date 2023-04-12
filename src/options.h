/*  options.h:
 *    structures containing different startup and runtime options
 *
 */

#ifndef __OPTIONS_H
#define __OPTIONS_H

#include <stdint.h>

class Options {
  private:
    /*  Bitflags for how to interact with the process
     *    0 = start in detached mode
     *    1 = open process with id (userland)
     *    2 = wrap process 
     */
    uint8_t m_attach_mode = 0;

  public:
    
};

#endif /* __OPTIONS_H */
