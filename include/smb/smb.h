////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2023 Matthew Deutsch
//
// Static is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// Static is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Static; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////

#ifndef STATIC_SMB_SMB_HEADER
#define STATIC_SMB_SMB_HEADER

#include "nes/nes.h"
#include "nes/ppux.h"

namespace sta::smb
{

inline constexpr size_t BASE_ROM_SIZE = 40976;
inline constexpr std::array<uint8_t, 16> BASE_ROM_MD5 = {
    0x81, 0x1b, 0x02, 0x7e, 0xaf, 0x99, 0xc2, 0xde,
    0xf7, 0xb9, 0x33, 0xc5, 0x20, 0x86, 0x36, 0xde,
};

// Some of the most relevant RAM addresses in SMB
enum RamAddress : uint16_t
{
    GAME_ENGINE_SUBROUTINE  = 0x000e,
    AREA_DATA               = 0x00e7,
    AREA_DATA_LOW           = 0x00e7,
    AREA_DATA_HIGH          = 0x00e8,
    SPRITE_DATA             = 0x0200,
    SCREENEDGE_PAGELOC      = 0x071a,
    SCREENEDGE_X_POS        = 0x071c,
    HORIZONTAL_SCROLL       = 0x073f,
    INTERVAL_TIMER_CONTROL  = 0x077f,

    WORLD_NUMBER            = 0x075f,
    LEVEL_NUMBER            = 0x075c,

    OPER_MODE               = 0x0770,
    OPER_MODE_TASK          = 0x0772,
    PLAYER_X_SPEED          = 0x0057,

    BLOCK_BUFFER_1          = 0x0500,
    BLOCK_BUFFER_84_DISC    = 0x062a, // Extending threecreepios suggestion
    BLOCK_BUFFER_2          = 0x05d0,

    // https://github.com/Kolpa/FF1Bot/blob/master/emulator/luaScripts/SMB-HitBoxes.lua
    ENEMY_FLAG              = 0x000f,
    MARIO_BOUNDING_BOX      = 0x04ac, // ulx, uly, drx, dry
    ENEMY_BOUNDING_BOX      = 0x04b0,

    PAUSE_SOUND_QUEUE       = 0x00fa,
    AREA_MUSIC_QUEUE        = 0x00fb,
    EVENT_MUSIC_QUEUE       = 0x00fc,
    NOISE_SOUND_QUEUE       = 0x00fd,
    SQUARE2_SOUND_QUEUE     = 0x00fe,
    SQUARE1_SOUND_QUEUE     = 0x00ff,
};

// Area IDs
//  (AREA_DATA_HIGH << 8) + AREA_DATA_LOW
// Define what is typically considered a 'level' like 1-1 is GROUD_AREA_6 for example.
// But also they are often re-used / shared.
enum class AreaID : uint16_t
{
    WATER_AREA_1        = 0xAE08,
    WATER_AREA_2        = 0xAE47,
    WATER_AREA_3        = 0xAEC2,
    GROUND_AREA_1       = 0xA46D,
    GROUND_AREA_2       = 0xA4D0,
    GROUND_AREA_3       = 0xA539,
    GROUND_AREA_4       = 0xA58C,
    GROUND_AREA_5       = 0xA61B,
    GROUND_AREA_6       = 0xA690, // the start
    GROUND_AREA_7       = 0xA6F5,
    GROUND_AREA_8       = 0xA74A,
    GROUND_AREA_9       = 0xA7CF,
    GROUND_AREA_10      = 0xA834,
    GROUND_AREA_11      = 0xA83D,
    GROUND_AREA_12      = 0xA87C,
    GROUND_AREA_13      = 0xA891,
    GROUND_AREA_14      = 0xA8F8,
    GROUND_AREA_15      = 0xA95D,
    GROUND_AREA_16      = 0xA9D0,
    GROUND_AREA_17      = 0xAA01,
    GROUND_AREA_18      = 0xAA94,
    GROUND_AREA_19      = 0xAB07,
    GROUND_AREA_20      = 0xAB80,
    GROUND_AREA_21      = 0xABD9,
    GROUND_AREA_22      = 0xAC04,
    UNDERGROUND_AREA_1  = 0xAC37,
    UNDERGROUND_AREA_2  = 0xACDA,
    UNDERGROUND_AREA_3  = 0xAD7B,
    CASTLE_AREA_1       = 0xA1B1,
    CASTLE_AREA_2       = 0xA212,
    CASTLE_AREA_3       = 0xA291,
    CASTLE_AREA_4       = 0xA304,
    CASTLE_AREA_5       = 0xA371,
    CASTLE_AREA_6       = 0xA3FC,
};
const std::vector<AreaID>& KnownAreaIDs();
AreaID AreaIDFromRAM(uint8_t area_data_low, uint8_t area_data_high);
std::string ToString(AreaID area_id);

// Sound effects
//  From queues (SQUARE1_SOUND_QUEUE, SQUARE2_SOUND_QUEUE, NOISE_SOUND_QUEUE, PAUSE_SOUND_QUEUE)
enum class SoundEffect : uint32_t
{
    SILENCE             = 0b00000000000000000000000000000000,

    SMALL_JUMP          = 0b00000000000000000000000010000000,
    FLAGPOLE            = 0b00000000000000000000000001000000,
    FIREBALL            = 0b00000000000000000000000000100000,
    PIPEDOWN_INJURY     = 0b00000000000000000000000000010000,
    ENEMY_SMACK         = 0b00000000000000000000000000001000,
    ENEMY_STOMP_SWIM    = 0b00000000000000000000000000000100,
    BUMP                = 0b00000000000000000000000000000010,
    BIG_JUMP            = 0b00000000000000000000000000000001,

    BOWSER_FALL         = 0b00000000000000001000000000000000,
    EXTRA_LIFE          = 0b00000000000000000100000000000000,
    GRAB_POWER_UP       = 0b00000000000000000010000000000000,
    TIMERTICK           = 0b00000000000000000001000000000000, // 4 frames in between
    BLAST               = 0b00000000000000000000100000000000, // fireworks and bullet bills
    GROW_VINE           = 0b00000000000000000000010000000000,
    GROW_POWER_UP       = 0b00000000000000000000001000000000,
    GRAB_COIN           = 0b00000000000000000000000100000000,

    BOWSER_FLAME        = 0b00000000100000000000000000000000,
    BRICK_SHATTER       = 0b00000000010000000000000000000000,

    DEATH_SOUND         = 0b00000001000000000000000000000000,
};
const std::vector<SoundEffect>& AudibleSoundEffects();
std::string ToString(SoundEffect effect);

// Music tracks
//  From queues (AREA_MUSIC_QUEUE, EVENT_MUSIC_QUEUE)
enum class MusicTrack : uint32_t
{
    SILENCE             = 0b00000000000000000000000000000000,

    // most of these tracks repeat
    STAR_POWER          = 0b00000000000000000000000001000000,
    PIPE_INTRO          = 0b00000000000000000000000000100000, // does not repeat
    CLOUD               = 0b00000000000000000000000000010000,
    CASTLE              = 0b00000000000000000000000000001000,
    UNDERGROUND         = 0b00000000000000000000000000000100,
    WATER               = 0b00000000000000000000000000000010,
    GROUND              = 0b00000000000000000000000000000001,

    // most of these tracks do not repeat
    TIME_RUNNING_OUT    = 0b00000000000000000100000000000000,
    END_OF_LEVEL        = 0b00000000000000000010000000000000, // repeats
    ALT_GAME_OVER       = 0b00000000000000000001000000000000,
    END_OF_CASTLE       = 0b00000000000000000000100000000000,
    VICTORY             = 0b00000000000000000000010000000000,
    GAME_OVER           = 0b00000000000000000000001000000000,
    DEATH_MUSIC         = 0b00000000000000000000000100000000,
};
const std::vector<MusicTrack>& AudibleMusicTracks();
std::string ToString(MusicTrack track);

// TODO bruh
bool AreaIDEnd(AreaID area_id, int* end);
bool AreaIDMusic(AreaID area_id, MusicTrack* track);


// Game end state
//  OPER_MODE == 2, OPER_MODE_TASK == 0x00, PLAYER_X_SPEED == 0x18

// Minimap is just a 2bpp image of the same size as a normal nes frame
inline constexpr int MINIMAP_NUM_BYTES = nes::FRAME_SIZE / 4;
inline constexpr int MINIMAP_RGB_PALETTE_SIZE = 4 * nes::BYTES_PER_PALETTE_ENTRY;
typedef std::array<uint8_t, 4> MinimapPalette; // palette entries into a nes palette
typedef std::array<uint8_t, MINIMAP_RGB_PALETTE_SIZE> MinimapPaletteBGR;

const MinimapPalette& DefaultMinimapPalette(); // indices
const MinimapPaletteBGR& DefaultMinimapPaletteBGR(); // bgr

typedef std::array<uint8_t, MINIMAP_NUM_BYTES> MinimapImage;


// For background stuff
struct SMBNametableDiff
{
    int NametablePage;
    int Offset;
    uint8_t Value;
};

class INametableCache
{
public:
    INametableCache(const uint8_t* smb_chr1)
        : m_smb_chr1(smb_chr1) {}
    ~INametableCache() = default;

    virtual const nes::NameTable* Nametable(AreaID id, int page) const = 0;
    virtual const MinimapImage* Minimap(AreaID id, int page) const = 0;

    // TODO fix api (leaving same as rgms for now)
    void RenderTo(AreaID id, int apx, int width, nes::PPUx* ppux, int x,
            const nes::Palette& pal = nes::DefaultPaletteBGR(),
            const uint8_t* pt = nullptr,
            const MinimapPalette* minimap = nullptr, // make nonnull to render minimap instead
            const uint8_t* fpal = nullptr, // make nonnull to overwrite found nametable
            const std::vector<SMBNametableDiff>* diffs = nullptr) const; // first diffs is priority

private:
    const uint8_t* m_smb_chr1;
    mutable nes::NameTable BufferTable;
};


// Part of the world
struct WorldSection
{
    AreaID AID;

    // World-Level (like 5-2)
    int World;
    int Level;

    // Pixel extents of this section (in world coordinates)
    int Left; // >=
    int Right; // <
    int Width() const {
        return Right - Left;
    }
    int LeftPage() const {
        return Left / 256;
    }
    int RightPage() const {
        return Right / 256 + 1;
    }
    int PageLeft(int p) {
        int pxl = static_cast<int>(p) * 256;
        if (pxl < Left) {
            return std::min(Left - pxl, 256);
        }
        return 0;
    }
    int PageRight(int p) {
        int pxl = static_cast<int>(p) * 256;
        int pxr = static_cast<int>(p) * 256 + 256;
        if (pxr > Right) {
            return std::max(Right - pxl, 0);
        }
        return 256;
    }

    // Location from farthest left (or relative to ppux)
    int XLoc;
    size_t SectionIndex;
};
inline bool operator==(const WorldSection& a, const WorldSection& b) {
    return a.AID == b.AID && a.World == b.World && a.Level == b.Level;
}

class Route
{
public:
    typedef std::vector<WorldSection>::iterator iterator;
    typedef std::vector<WorldSection>::const_iterator const_iterator;

    Route() = default;
    ~Route() = default;

    bool empty() const {
        return m_Route.empty();
    }
    WorldSection& emplace_back() {
        return m_Route.emplace_back();
    }
    WorldSection& back() {
        return m_Route.back();
    }
    void clear() {
        m_Route.clear();
    }
    iterator begin() {
        return m_Route.begin();
    }
    const_iterator begin() const {
        return m_Route.begin();
    }
    iterator end() {
        return m_Route.end();
    }
    const_iterator end() const {
        return m_Route.end();
    }
    WorldSection& operator[](size_t i) {
        return m_Route[i];
    }
    const WorldSection& operator[](size_t i) const {
        return m_Route[i];
    }
    WorldSection& at(size_t i) {
        return m_Route.at(i);
    }
    const WorldSection& at(size_t i) const {
        return m_Route.at(i);
    }
    size_t size() const {
        return m_Route.size();
    }


    int TotalWidth() const {
        if (m_Route.empty()) {
            return 0;
        }
        return m_Route.back().XLoc + m_Route.back().Width();
    }
    int last_x(int width) const {
        return std::max(TotalWidth() - width, 0);
    }

    void GetVisibleSections(int xloc, int width,
            std::vector<WorldSection>* sections) const;

    // TODO rgms...
    void RenderMinimapTo(nes::PPUx* ppux, int categoryX,
            const MinimapPalette& minimapPalette,
            const INametableCache* nametables,
            std::vector<WorldSection>* visibleSections) const;
    bool InCategory(AreaID id, int apx, int world, int level,
            int* categoryX, int* sectionIndex) const;

    const std::vector<WorldSection>& Sections() const {
        return m_Route;
    }
private:
    std::vector<WorldSection> m_Route;
};


// TODO These should be somewhere else? and fix (rgms...)
void RenderMinimapToPPUx(int x, int y, int sx, int ex,
        const MinimapImage& img, const MinimapPalette& miniPal,
        const nes::Palette& nesPal, nes::PPUx* ppux);
void RenderMinimapToPPUx(int x, int y, const nes::EffectInfo& effects,
        const MinimapImage& img, const MinimapPalette& miniPal,
        const nes::Palette& nesPal, nes::PPUx* ppux);
bool IsMarioTile(uint8_t tile_index);

}

#endif
