//
// Created by felix on 6/26/19.
//

#include <time.h>
#include "CCTools.h"

long CCTools::GetTimestamp() {
    time_t t = time(0);
    return t;
}
