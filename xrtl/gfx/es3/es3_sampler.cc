// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "xrtl/gfx/es3/es3_sampler.h"

#include "xrtl/gfx/es3/es3_platform_context.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3Sampler::ES3Sampler(ES3ObjectLifetimeQueue* queue, Params params)
    : Sampler(params), queue_(queue) {
  queue_->EnqueueObjectAllocation(this);
}

ES3Sampler::~ES3Sampler() = default;

void ES3Sampler::PrepareAllocation() { queue_->EnqueueObjectAllocation(this); }

void ES3Sampler::Release() { queue_->EnqueueObjectDeallocation(this); }

bool ES3Sampler::AllocateOnQueue() {
  ES3PlatformContext::CheckHasContextLock();

  // TODO(benvanik): pool ID allocation.
  glGenSamplers(1, &sampler_id_);

  static const GLenum kTextureWrapMap[4] = {
      GL_REPEAT,           // kRepeat
      GL_MIRRORED_REPEAT,  // kMirroredRepeat
      GL_CLAMP_TO_EDGE,    // kClampToEdge
      GL_CLAMP_TO_EDGE,    // kClampToBorder (not supported?)
  };
  glSamplerParameteri(
      sampler_id_, GL_TEXTURE_WRAP_S,
      kTextureWrapMap[static_cast<int>(params_.address_mode_u)]);
  glSamplerParameteri(
      sampler_id_, GL_TEXTURE_WRAP_T,
      kTextureWrapMap[static_cast<int>(params_.address_mode_v)]);
  glSamplerParameteri(
      sampler_id_, GL_TEXTURE_WRAP_R,
      kTextureWrapMap[static_cast<int>(params_.address_mode_w)]);

  GLint min_filter = GL_NEAREST;
  switch (params_.min_filter) {
    case Filter::kNearest:
      switch (params_.mipmap_mode) {
        case MipmapMode::kNearest:
          min_filter = GL_NEAREST_MIPMAP_NEAREST;
          break;
        case MipmapMode::kLinear:
          min_filter = GL_NEAREST_MIPMAP_LINEAR;
          break;
      }
      break;
    case Sampler::Filter::kLinear:
      switch (params_.mipmap_mode) {
        case MipmapMode::kNearest:
          min_filter = GL_LINEAR_MIPMAP_NEAREST;
          break;
        case MipmapMode::kLinear:
          min_filter = GL_LINEAR_MIPMAP_LINEAR;
          break;
      }
      break;
  }
  glSamplerParameteri(sampler_id_, GL_TEXTURE_MIN_FILTER, min_filter);

  GLint mag_filter = GL_NEAREST;
  switch (params_.mag_filter) {
    case Filter::kNearest:
      mag_filter = GL_NEAREST;
      break;
    case Filter::kLinear:
      mag_filter = GL_LINEAR;
      break;
  }
  glSamplerParameteri(sampler_id_, GL_TEXTURE_MAG_FILTER, mag_filter);

  glSamplerParameterf(sampler_id_, GL_TEXTURE_MIN_LOD, params_.min_lod);
  glSamplerParameterf(sampler_id_, GL_TEXTURE_MAX_LOD, params_.max_lod);

  // TODO(benvanik): params.mip_lod_bias
  // TODO(benvanik): params.anisotropy_enable
  // TODO(benvanik): params.max_anisotropy
  // TODO(benvanik): params.border_color

  return true;
}

void ES3Sampler::DeallocateOnQueue() {
  ES3PlatformContext::CheckHasContextLock();
  if (sampler_id_) {
    glDeleteSamplers(1, &sampler_id_);
    sampler_id_ = 0;
  }
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
