#include "animation/animation_system.h"#include "core/memory.h"static AnimationSystem* anim_system = NULL;void anim_init_system(void) {    anim_system = (AnimationSystem*)mem_alloc(sizeof(AnimationSystem));    anim_system->current_time = 0.0f;    anim_system->animations = NULL;    anim_system->anim_count = 0;}void anim_update(float delta_time) {    if (!anim_system) return;        anim_system->current_time += delta_time;        for (uint32_t i = 0; i < anim_system->anim_count; i++) {        Animation* anim = &anim_system->animations[i];        if (!anim->playing) continue;                // Atualiza frame baseado no tempo        float frame_time = anim_system->current_time * anim->speed_multiplier;        anim->current_frame = (uint32_t)(frame_time / anim->frame_duration) % anim->frame_count;                // Notifica listeners        if (anim->on_frame_change) {            anim->on_frame_change(anim, anim->current_frame);        }    }}