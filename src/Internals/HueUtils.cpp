#include "HueUtils.h"

namespace Hueduino {
    namespace Internals {
        int searchStr(const char* str, const char* arr[], long arr_len) {
            if(!str || !*str || arr_len < 1) return -1;

            long left = 0;
            long right = arr_len-1;

            while(left <= right) {
                size_t middle = (left+right)/2;

                const char* type_str = arr[middle];
                const char* str_p = str;
                while(*type_str && *str_p && *str_p == *type_str){
                    str_p++;
                    type_str++;
                }

                if(*str_p > *type_str) left = middle+1;
                else if(*str_p < *type_str) right = middle-1;
                else return middle;
            }

            return -1;
        }
    } // Internals
    
} // Hueduino