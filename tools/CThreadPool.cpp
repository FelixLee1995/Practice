//
// Created by felix on 6/26/19.
//

#include "CThreadPool.h"
#include <vector>


int minNumberInRotateArray(std::vector<int> rotateArray) {
    if (rotateArray.size() == 0) {
        return 0;
    }

    for (int i = rotateArray.size()-1;i>0;i--) {
        if (rotateArray[i]< rotateArray[i-1]) {
            return rotateArray[i];
        }
    }
}
