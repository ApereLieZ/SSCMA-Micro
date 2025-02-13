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

#include "el_algorithm_dirnet.h"

#include <type_traits>

#include "core/el_debug.h"
#include "core/utils/el_cv.h"

namespace edgelab {

AlgorithmDirNet::InfoType AlgorithmDirNet::algorithm_info{el_algorithm_dirnet_config_t::info};

AlgorithmDirNet::AlgorithmDirNet(EngineType* engine) : Algorithm(engine, AlgorithmDirNet::algorithm_info) {
    init();
}

AlgorithmDirNet::AlgorithmDirNet(EngineType* engine, const ConfigType& config) : Algorithm(engine, config.info) {
    init();
}

AlgorithmDirNet::~AlgorithmDirNet() { _results.clear(); }

bool AlgorithmDirNet::is_model_valid(const EngineType* engine) {
    const auto& input_shape{engine->get_input_shape(0)};
    if (input_shape.size != 4 ||      // B, W, H, C
        input_shape.dims[0] != 1 ||   // B = 1
        input_shape.dims[1] < 16 ||   // W >= 16
        input_shape.dims[2] < 16 ||   // H >= 16
        (input_shape.dims[3] != 3))   // C = RGB (that is, 3))
        return false;

    const auto& output_shape{engine->get_output_shape(0)};
    if (output_shape.size != 2 ||     // B, (X, Y)
        output_shape.dims[0] != 1 ||  // B = 1
        output_shape.dims[1] == 2     // (X, Y) == 2 
    )
        return false;

    return true;
}

inline void AlgorithmDirNet::init() {
    EL_ASSERT(is_model_valid(this->__p_engine));

    _input_img.data   = static_cast<decltype(ImageType::data)>(this->__p_engine->get_input(0));
    _input_img.width  = static_cast<decltype(ImageType::width)>(this->__input_shape.dims[1]),
    _input_img.height = static_cast<decltype(ImageType::height)>(this->__input_shape.dims[2]),
    _input_img.size =
      static_cast<decltype(ImageType::size)>(_input_img.width * _input_img.height * this->__input_shape.dims[3]);
    _input_img.format = EL_PIXEL_FORMAT_RGB888;
    _input_img.rotate = EL_PIXEL_ROTATE_0;

    EL_ASSERT(_input_img.format != EL_PIXEL_FORMAT_UNKNOWN);
    EL_ASSERT(_input_img.rotate != EL_PIXEL_ROTATE_UNKNOWN);
}

el_err_code_t AlgorithmDirNet::run(ImageType* input) {
    // TODO: image type conversion before underlying_run, because underlying_run doing a type erasure
    return underlying_run(input);
};

el_err_code_t AlgorithmDirNet::preprocess() {
    auto* i_img{static_cast<ImageType*>(this->__p_input)};

    // convert image
    el_img_convert(i_img, &_input_img);

    auto size{_input_img.size};
    for (decltype(ImageType::size) i{0}; i < size; ++i) {
        _input_img.data[i] -= 128;
    }

    return EL_OK;
}

el_err_code_t AlgorithmDirNet::postprocess() {
    _results.clear();

    // get output
    auto* data{static_cast<int8_t*>(this->__p_engine->get_output(0))};

    float scale{this->__output_quant.scale};
    bool  rescale{scale < 0.1f ? true : false};

    int32_t zero_point{this->__output_quant.zero_point};

    auto x{static_cast<decltype(scale)> (data[0] - zero_point) * scale};
    auto y{static_cast<decltype(scale)> (data[1] - zero_point) * scale};
    
    _results.emplace_front(OutputType{.x = static_cast<decltype(OutputType::x)>(x),
                            .y = static_cast<decltype(OutputType::y)>(y)});

    return EL_OK;
}

const std::forward_list<AlgorithmDirNet::OutputType>& AlgorithmDirNet::get_results() const { return _results; }

void AlgorithmDirNet::set_algorithm_config(const ConfigType& config) {}

AlgorithmDirNet::ConfigType AlgorithmDirNet::get_algorithm_config() const {
    ConfigType config;
    return config;
}

}  // namespace edgelab
