/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Seeed Technology Co.,Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <forward_list>

#include "core/el_types.h"
#include "el_algorithm_base.h"

namespace edgelab {

using namespace edgelab::base;
using namespace edgelab::types;

namespace types {

// we're not using inheritance since it not standard layout
struct el_algorithm_dirnet_config_t {
    static constexpr el_algorithm_info_t info{
      .type = EL_ALGO_TYPE_DIRNET, .categroy = EL_ALGO_CAT_DIRNET, .input_from = EL_SENSOR_TYPE_CAM};
};

}  // namespace types

class AlgorithmDirNet : public Algorithm {
   public:
    using ImageType  = el_img_t;
    using OutputType  = el_direction_t;
    using ConfigType = el_algorithm_dirnet_config_t;

    static InfoType algorithm_info;

    AlgorithmDirNet(EngineType* engine);
    AlgorithmDirNet(EngineType* engine, const ConfigType& config);
    ~AlgorithmDirNet();

    static bool is_model_valid(const EngineType* engine);

    el_err_code_t                       run(ImageType* input);
    const std::forward_list<OutputType>& get_results() const;

    void       set_algorithm_config(const ConfigType& config);
    ConfigType get_algorithm_config() const;

   protected:
    inline void init();

    el_err_code_t preprocess() override;
    el_err_code_t postprocess() override;

   private:
    ImageType _input_img;

    std::forward_list<OutputType> _results;
};

}  // namespace edgelab
