// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <algorithm>
#include <array>
#include <span>
#include <bc_decoder.h>

#include "common/common_types.h"
#include "video_core/texture_cache/decode_bc.h"

namespace VideoCommon {

namespace {
constexpr u32 BLOCK_SIZE = 4;

using VideoCore::Surface::PixelFormat;

constexpr bool IsSigned(PixelFormat pixel_format) {
    switch (pixel_format) {
    case PixelFormat::BC4_SNORM:
    case PixelFormat::BC4_UNORM:
    case PixelFormat::BC5_SNORM:
    case PixelFormat::BC5_UNORM:
    case PixelFormat::BC6H_SFLOAT:
    case PixelFormat::BC6H_UFLOAT:
        return true;
    default:
        return false;
    }
}

constexpr u32 BlockSize(PixelFormat pixel_format) {
    switch (pixel_format) {
    case PixelFormat::BC1_RGBA_SRGB:
    case PixelFormat::BC1_RGBA_UNORM:
    case PixelFormat::BC4_SNORM:
    case PixelFormat::BC4_UNORM:
        return 8;
    default:
        return 16;
    }
}
} // Anonymous namespace

u32 ConvertedBytesPerBlock(VideoCore::Surface::PixelFormat pixel_format) {
    switch (pixel_format) {
    case PixelFormat::BC4_SNORM:
    case PixelFormat::BC4_UNORM:
        return 1;
    case PixelFormat::BC5_SNORM:
    case PixelFormat::BC5_UNORM:
        return 2;
    case PixelFormat::BC6H_SFLOAT:
    case PixelFormat::BC6H_UFLOAT:
        return 8;
    default:
        return 4;
    }
}

template <auto decompress, PixelFormat pixel_format>
void DecompressBlocks(std::span<const u8> input, std::span<u8> output, Extent3D extent,
                      bool is_signed = false) {
    const u32 out_bpp = ConvertedBytesPerBlock(pixel_format);
    const u32 block_width = std::min(extent.width, BLOCK_SIZE);
    const u32 block_height = std::min(extent.height, BLOCK_SIZE);
    const u32 pitch = extent.width * out_bpp;
    size_t input_offset = 0;
    size_t output_offset = 0;
    for (u32 slice = 0; slice < extent.depth; ++slice) {
        for (u32 y = 0; y < extent.height; y += block_height) {
            size_t row_offset = 0;
            for (u32 x = 0; x < extent.width;
                 x += block_width, row_offset += block_width * out_bpp) {
                const u8* src = input.data() + input_offset;
                u8* const dst = output.data() + output_offset + row_offset;
                if constexpr (IsSigned(pixel_format)) {
                    decompress(src, dst, x, y, extent.width, extent.height, is_signed);
                } else {
                    decompress(src, dst, x, y, extent.width, extent.height);
                }
                input_offset += BlockSize(pixel_format);
            }
            output_offset += block_height * pitch;
        }
    }
}

void DecompressBCn(std::span<const u8> input, std::span<u8> output, Extent3D extent,
                   VideoCore::Surface::PixelFormat pixel_format) {
    switch (pixel_format) {
    case PixelFormat::BC1_RGBA_UNORM:
    case PixelFormat::BC1_RGBA_SRGB:
        DecompressBlocks<bcn::DecodeBc1, PixelFormat::BC1_RGBA_UNORM>(input, output, extent);
        break;
    case PixelFormat::BC2_UNORM:
    case PixelFormat::BC2_SRGB:
        DecompressBlocks<bcn::DecodeBc2, PixelFormat::BC2_UNORM>(input, output, extent);
        break;
    case PixelFormat::BC3_UNORM:
    case PixelFormat::BC3_SRGB:
        DecompressBlocks<bcn::DecodeBc3, PixelFormat::BC3_UNORM>(input, output, extent);
        break;
    case PixelFormat::BC4_SNORM:
    case PixelFormat::BC4_UNORM:
        DecompressBlocks<bcn::DecodeBc4, PixelFormat::BC4_UNORM>(
            input, output, extent, pixel_format == PixelFormat::BC4_SNORM);
        break;
    case PixelFormat::BC5_SNORM:
    case PixelFormat::BC5_UNORM:
        DecompressBlocks<bcn::DecodeBc5, PixelFormat::BC5_UNORM>(
            input, output, extent, pixel_format == PixelFormat::BC5_SNORM);
        break;
    case PixelFormat::BC6H_SFLOAT:
    case PixelFormat::BC6H_UFLOAT:
        DecompressBlocks<bcn::DecodeBc6, PixelFormat::BC6H_UFLOAT>(
            input, output, extent, pixel_format == PixelFormat::BC6H_SFLOAT);
        break;
    case PixelFormat::BC7_SRGB:
    case PixelFormat::BC7_UNORM:
        DecompressBlocks<bcn::DecodeBc7, PixelFormat::BC7_UNORM>(input, output, extent);
        break;
    default:
        LOG_WARNING(HW_GPU, "Unimplemented BCn decompression {}", pixel_format);
    }
}

} // namespace VideoCommon
