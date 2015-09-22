/******************************************************************************
 * Copyright 2015 ASDF developers
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
/**
 * @file private/common.h
 * @brief
 * @author Matthieu Lefebvre
 */

#ifndef _ASDF_PRIVATE_COMMON_H_
#define _ASDF_PRIVATE_COMMON_H_

#define CHK_H5(X) \
    do { \
        int ret__ = (X); \
        if (ret__ < 0) \
            return ret__; \
    } while (0)

#endif
