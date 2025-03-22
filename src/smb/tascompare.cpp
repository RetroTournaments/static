
#include <unordered_set>
#include <fstream>
#include <array>

#include "ext/nfdext/nfdext.h"
#include "smb/tascompare.h"
#include "smb/smbdbui.h"
#include "nes/nes.h"
#include "util/file.h"
#include "nes/nestopiaimpl.h"

using namespace sta;
using namespace sta::smbui;
using namespace sta::rgms;
using namespace sta::nes;

static void EmuToOutput(int frameIndex, int* startIndex, nes::ControllerState cs,
        const nes::NestopiaNESEmulator& emu, rgms::SMBMessageProcessorOutput* out,
        const smb::SMBNametableCachePtr& nametables)
{
    using namespace smb;

    out->ConsolePoweredOn = true;
    double mult = (1.0 / nes::NTSC_FPS) * 1000 * (1 / nes::NTSC_MS_PER_M2);
    out->M2Count = frameIndex * mult;
    if (startIndex && *startIndex != -1) {
        out->UserM2 = (frameIndex - *startIndex) * mult;
    } else {
        out->UserM2 = 0;
    }

    out->Controller = cs;
    emu.PPUPeekFramePalette(&out->FramePalette);

    AreaID aid = smb::AreaIDFromRAM(
            emu.CPUPeek(RamAddress::AREA_DATA_LOW),
            emu.CPUPeek(RamAddress::AREA_DATA_HIGH));
    int apx = rgms::AreaPointerXFromData(
            emu.CPUPeek(RamAddress::SCREENEDGE_PAGELOC),
            emu.CPUPeek(RamAddress::SCREENEDGE_X_POS),
            aid,
            emu.CPUPeek(RamAddress::BLOCK_BUFFER_84_DISC));

    out->Frame.AID = aid;
    out->Frame.PrevAPX = apx;
    out->Frame.APX = apx;
    out->Frame.IntervalTimerControl = emu.CPUPeek(RamAddress::INTERVAL_TIMER_CONTROL);
    out->Frame.GameEngineSubroutine = emu.CPUPeek(RamAddress::GAME_ENGINE_SUBROUTINE);
    out->Frame.OperMode = emu.CPUPeek(RamAddress::OPER_MODE);

    out->Frame.Time = static_cast<int>(emu.PPUPeek8(0x2000 + 0x007a)) * 100 +
                      static_cast<int>(emu.PPUPeek8(0x2000 + 0x007b)) * 10 +
                      static_cast<int>(emu.PPUPeek8(0x2000 + 0x007c)) * 1;

    out->Frame.World = emu.CPUPeek(RamAddress::WORLD_NUMBER) + 0x01;
    out->Frame.Level = emu.CPUPeek(RamAddress::LEVEL_NUMBER) + 0x01;

    auto PPUNT0At = [&](int x, int y){
        return emu.PPUPeek8(0x2000 + x + y * nes::NAMETABLE_WIDTH_BYTES);
    };

    for (int i = 0; i < out->Frame.TitleScreen.ScoreTiles.size(); i++) {
        out->Frame.TitleScreen.ScoreTiles[i] = PPUNT0At(TITLESCREEN_SCORE_X + i, TITLESCREEN_SCORE_Y);
    }
    out->Frame.TitleScreen.CoinTiles[0] = PPUNT0At(TITLESCREEN_COIN_X + 0, TITLESCREEN_COIN_Y);
    out->Frame.TitleScreen.CoinTiles[1] = PPUNT0At(TITLESCREEN_COIN_X + 1, TITLESCREEN_COIN_Y);

    out->Frame.TitleScreen.WorldTile = PPUNT0At(TITLESCREEN_WORLD_X, TITLESCREEN_WORLD_Y);
    out->Frame.TitleScreen.LevelTile = PPUNT0At(TITLESCREEN_LEVEL_X, TITLESCREEN_LEVEL_Y);

    out->Frame.TitleScreen.LifeTiles[0] = PPUNT0At(TITLESCREEN_LIFE_X + 0, TITLESCREEN_LIFE_Y);
    out->Frame.TitleScreen.LifeTiles[1] = PPUNT0At(TITLESCREEN_LIFE_X + 1, TITLESCREEN_LIFE_Y);

    int lpage = apx / 256;
    int rpage = lpage + 1;
    if ((apx % 512) >= 256) {
        std::swap(lpage, rpage);
    }

    out->Frame.NTDiffs.clear();
    out->Frame.TopRows.clear();

    std::array<const sta::smb::db::nametable_page*, 2> nts = {nullptr, nullptr};
    nts[0] = nametables->MaybeGetNametable(aid, lpage);
    nts[1] = nametables->MaybeGetNametable(aid, rpage);

    auto ntpeek = [&](int i, int j){
        return emu.PPUPeek8(0x2000 + 0x400 * i + j);
    };

    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            for (int j = 0; j < 32 * 4; j++) {
                int y = j / nes::NAMETABLE_WIDTH_BYTES;
                int x = j % nes::NAMETABLE_WIDTH_BYTES;
                if (x <= 1 || x >= 30 || y <= 1) {
                    out->Frame.TopRows.push_back(36); // yolo
                } else {
                    out->Frame.TopRows.push_back(ntpeek(i, j));
                }
            }
            for (int j = 0; j < 32; j++) {
                out->Frame.TopRows.push_back(ntpeek(i, j + nes::NAMETABLE_ATTRIBUTE_OFFSET));
            }
        }

        if (nts[i]) {
            std::unordered_set<int> diffAttrs;
            for (int j = 32*4; j < nes::NAMETABLE_ATTRIBUTE_OFFSET; j++) {
                if (ntpeek(i, j) != nts[i]->nametable[j]) {
                    int y = j / nes::NAMETABLE_WIDTH_BYTES;
                    int x = j % nes::NAMETABLE_WIDTH_BYTES;

                    int tapx = (x * 8) + (nts[i]->page * 256);
                    if ((tapx > (apx - 8)) && (tapx < (apx + 256))) {
                        smb::SMBNametableDiff diff;
                        diff.NametablePage = nts[i]->page;
                        diff.Offset = j;
                        diff.Value = ntpeek(i, j);

                        out->Frame.NTDiffs.push_back(diff);

                        int cy = y / 4;
                        int cx = x / 4;

                        int attrIndex = nes::NAMETABLE_ATTRIBUTE_OFFSET + cy * (nes::NAMETABLE_WIDTH_BYTES / 4) + cx;
                        if (ntpeek(i, attrIndex) != nts[i]->nametable[attrIndex]) {
                            diffAttrs.insert(attrIndex);
                        }

                    }
                }

                for (auto & attrIndex : diffAttrs) {
                    smb::SMBNametableDiff diff;
                    diff.NametablePage = nts[i]->page;
                    diff.Offset = attrIndex;
                    diff.Value = ntpeek(i, attrIndex);

                    out->Frame.NTDiffs.push_back(diff);
                }

            }
        }
    }

    const uint8_t* fpal = out->FramePalette.data();
    for (int i = 0; i < nes::NUM_OAM_ENTRIES; i++) {
        if (i == 0) { // Skip sprite zero, it's always the bottom of the coin
            continue;
        }
        uint8_t y = emu.CPUPeek(RamAddress::SPRITE_DATA + (i * 4) + 0);
        uint8_t tile_index = emu.CPUPeek(RamAddress::SPRITE_DATA + (i * 4) + 1);
        uint8_t attributes = emu.CPUPeek(RamAddress::SPRITE_DATA + (i * 4) + 2);
        uint8_t x = emu.CPUPeek(RamAddress::SPRITE_DATA + (i * 4) + 3);

        if (y > 240) { // 'off screen'
            continue;
        }

        nes::OAMxEntry oamx;
        oamx.X = static_cast<int>(x);
        oamx.Y = static_cast<int>(y);
        oamx.TileIndex = tile_index;
        oamx.Attributes = attributes;
        oamx.PatternTableIndex = 0;

        oamx.TilePalette[0] = fpal[16];
        uint8_t p = attributes & nes::OAM_PALETTE;
        for (int j = 1; j < 4; j++) {
            oamx.TilePalette[j] = fpal[16 + p * 4 + j];
        }

        out->Frame.OAMX.push_back(oamx);
    }

    if (startIndex) {
        if (*startIndex == -1 && aid == smb::AreaID::GROUND_AREA_6 && apx < 15 && out->Frame.Time == 400) {
            *startIndex = frameIndex;
        }
    }
}


static bool InitTas(smb::SMBDatabase* database, const std::string& path, TasCompareTAS* tas, size_t min = 1) {
    tas->name = "";
    tas->path = path;
    tas->start = 0;
    tas->opacity = 1.0f;

    auto rom = database->GetBaseRom();

    nes::NestopiaNESEmulator emu;
    emu.LoadINESData(rom->data(), rom->size());

    std::vector<nes::ControllerState> inputs;
    std::ifstream ifs(path);
    if (!ifs.good()) {
        std::cout << "no good?" << std::endl;
        return false;
    }
    nes::ReadFM2File(ifs, &inputs, nullptr);

    size_t n = std::max(inputs.size(), min);

    tas->outputs.resize(n);
    tas->frames.resize(n);

    int frame_index = 0;
    for (auto & input : inputs) {
        if (frame_index % 1000 == 0) {
            std::cout << path << " " << frame_index << std::endl;
        }
        emu.Execute(input);

        EmuToOutput(frame_index, &tas->start, input, emu, &tas->outputs[frame_index], database->GetNametableCache());
        emu.ScreenPeekFrame(&tas->frames[frame_index]);
        frame_index++;
    }
    while (frame_index < n) {
        if (frame_index % 1000 == 0) {
            std::cout << path << " " << frame_index << std::endl;
        }
        emu.Execute(0x00);

        EmuToOutput(frame_index, &tas->start, 0x00, emu, &tas->outputs[frame_index], database->GetNametableCache());
        emu.ScreenPeekFrame(&tas->frames[frame_index]);
        frame_index++;
    }
    return !inputs.empty();
}

static void SyncToArea(TasCompareInfo* info, smb::AreaID aid)
{
    for (auto & tas : info->tases) {
        for (int i = 0; i < static_cast<int>(tas.frames.size()); i++) {
            if (tas.outputs[i].Frame.AID == aid) {
                tas.start = i;
                break;
            }
        }
    }
}

static cv::Mat RenderShot(smb::SMBDatabase* db, TasCompareInfo* info, int shot_index, int frame_index)
{
    auto nts = db->GetNametableCache();
    auto rom = db->GetBaseRom();

    nes::RenderInfo render;
    render.OffX = 0;
    render.OffY = 0;
    render.Scale = 1;
    const uint8_t* chr1 = smb::rom_chr1(rom);
    render.PatternTables.push_back(smb::rom_chr0(rom));
    render.PatternTables.push_back(chr1);
    auto& nesPalette = nes::DefaultPaletteBGR();
    render.PaletteBGR = nesPalette.data();

    const auto& shot = info->shots[shot_index];

    nes::PPUx ppux(shot.width, nes::FRAME_HEIGHT, PPUxPriorityStatus::ENABLED);

    smb::AreaID aid = shot.aid;
    int apx = shot.apx;

    if (shot.follow_tas_index >= 0 && shot.follow_tas_index < info->tases.size()) {
        const auto& tas = info->tases[shot.follow_tas_index];
        int actual_frame = frame_index + tas.start;
        if (actual_frame >= 0 && actual_frame < tas.frames.size()) {
            auto& out = tas.outputs[actual_frame];
            aid = out.Frame.AID;
            apx = out.Frame.APX - (shot.width - nes::FRAME_WIDTH) / 2;
        }
    }

    std::vector<smb::SMBNametableDiff> diffs;
    bool set = false;
    nes::FramePalette fpal;
    for (auto& tas : info->tases) {
        int actual_frame = frame_index + tas.start;
        if (actual_frame >= 0 && actual_frame < tas.frames.size()) {
            auto& out = tas.outputs[actual_frame];
            if (out.Frame.AID == aid) {
                if (!set) {
                    set = true;
                    fpal = out.FramePalette;
                }
                diffs.insert(diffs.end(), out.Frame.NTDiffs.begin(), out.Frame.NTDiffs.end());
            }
        }
    }

    sta::smb::MinimapPalette mpal = smb::DefaultMinimapPalette();
    sta::smb::MinimapPalette* mpalp = nullptr;
    if (!set) {
        ppux.FillBackground(nes::PALETTE_ENTRY_WHITE, nes::DefaultPaletteBGR().data());
        mpalp = &mpal;
    } else {
        ppux.FillBackground(nes::PALETTE_ENTRY_BLACK, nes::DefaultPaletteBGR().data());
    }
    nts->RenderTo(aid, apx, shot.width, &ppux, 0,
            nes::DefaultPaletteBGR(), smb::rom_chr1(rom), mpalp, fpal.data(), &diffs);

    for (auto & tas : info->tases) {
        int actual_frame = frame_index + tas.start;
        if (actual_frame >= 0 && actual_frame < tas.frames.size()) {
            auto& out = tas.outputs[actual_frame];
            if (out.Frame.AID == aid) {
                for (auto oamx : out.Frame.OAMX) {
                    oamx.X += out.Frame.APX - apx;
                    auto effects = nes::EffectInfo::Defaults();
                    effects.Opacity = tas.opacity;
                    ppux.RenderOAMxEntry(oamx, render, effects);
                }
            }
        }
    }

    cv::Mat m(ppux.GetHeight(), ppux.GetWidth(), CV_8UC3, ppux.GetBGROut());
    return m.clone();
}

static void ExportShot(smb::SMBDatabase* db, TasCompareInfo* info, int shot_index, const std::string& name)
{
    std::string dir = "shot_temp/";
    util::fs::remove_all(dir);
    util::fs::create_directory(dir);

    int q = 0;
    for (int f = info->start_frame; f <= info->end_frame; f++) {
        cv::Mat m2 = RenderShot(db, info, shot_index, f);
        cv::resize(m2, m2, {}, 4, 4, cv::INTER_NEAREST);
        cv::imwrite(fmt::format("{}{:07d}.png", dir, q++), m2);
    }

    std::string cmd = fmt::format("ffmpeg -y -framerate 60 -i {}%07d.png -vcodec libx264 -crf 6 -vf format=yuv420p {}.mp4",
            dir, name);
    int r = system(cmd.c_str());
    std::cout << "ret: " << r << std::endl;
}

SMBTasCompareApplication::SMBTasCompareApplication(smb::SMBDatabase* db)
    : m_db(db)
{
    m_info.start_frame = 0;
    m_info.end_frame = 3500;
    m_info.current_frame = 0;
    RegisterComponent(std::make_shared<SMBTasCompareTases>(db, &m_info));
    RegisterComponent(std::make_shared<SMBTasCompareIndividual>(db, &m_info));
    RegisterComponent(std::make_shared<SMBTasCompareCombined>(db, &m_info));
}

SMBTasCompareApplication::~SMBTasCompareApplication()
{
}

bool SMBTasCompareApplication::OnFrame() {
    // ugh don't look at my bad code.
    if (ImGui::Begin("temp")) {
        if (ImGui::Button("export_kosmics_clips")) {
            int niftmin = 19200;

            m_info.shots.clear();
            {
                Shot s = {
                    .aid = smb::AreaID::CASTLE_AREA_6,
                    .apx = 0,
                    .width = 512,
                    .follow_tas_index = 1
                };
                m_info.shots.push_back(s);
            }
            for (int i = 0; i < 18; i++) {
                {
                    Shot s = {
                        .aid = smb::AreaID::CASTLE_AREA_6,
                        .apx = i * 256,
                        .width = 512,
                        .follow_tas_index = -1
                    };
                    m_info.shots.push_back(s);
                }
            }
            for (int i = 0; i < 4; i++) {
                {
                    Shot s = {
                        .aid = smb::AreaID::WATER_AREA_3,
                        .apx = i * 256,
                        .width = 512,
                        .follow_tas_index = -1
                    };
                    m_info.shots.push_back(s);
                }
            }

            auto do_with_other = [&](const std::string& other_path, const std::string& other_name){
                m_info.tases.clear();

                m_info.tases.emplace_back();
                InitTas(m_db, other_path, &m_info.tases.back(), niftmin);
                m_info.tases.back().opacity = 0.5;

                m_info.tases.emplace_back();
                InitTas(m_db, "/home/matthew/repos/static/data/smb/fm2/nift_454565.fm2", &m_info.tases.back(), niftmin);

                SyncToArea(&m_info, smb::AreaID::CASTLE_AREA_6);

                ExportShot(m_db, &m_info, 0, fmt::format("follow_454565_vs_{}", other_name));
                for (int i = 1; i < m_info.shots.size(); i++) {
                    ExportShot(m_db, &m_info, i, fmt::format("fixed_454565_vs_{}_{:04X}_{:02d}", other_name, static_cast<int>(m_info.shots[i].aid), m_info.shots[i].apx));
                }
            };

            //do_with_other("/home/matthew/repos/static/data/smb/fm2/tas_454265.fm2", "454265");
            //do_with_other("/home/matthew/repos/static/data/smb/fm2/nift_454631.fm2", "454631");
            //do_with_other("/home/matthew/repos/static/data/smb/fm2/nift_454798.fm2", "454798");

            //m_info.tases.clear();
            //m_info.tases.emplace_back();
            //InitTas(m_db, "/home/matthew/repos/static/data/smb/fm2/tas_454265.fm2", &m_info.tases.back(), niftmin);
            //m_info.tases.back().opacity = 0.7;
            //m_info.tases.emplace_back();
            //InitTas(m_db, "/home/matthew/repos/static/data/smb/fm2/nift_454798.fm2", &m_info.tases.back(), niftmin);
            //m_info.tases.back().opacity = 0.7;
            //m_info.tases.emplace_back();
            //InitTas(m_db, "/home/matthew/repos/static/data/smb/fm2/nift_454631.fm2", &m_info.tases.back(), niftmin);
            //m_info.tases.back().opacity = 0.7;
            //m_info.tases.emplace_back();
            //InitTas(m_db, "/home/matthew/repos/static/data/smb/fm2/nift_454565.fm2", &m_info.tases.back(), niftmin);
            //m_info.tases.back().opacity = 0.7;

            //SyncToArea(&m_info, smb::AreaID::CASTLE_AREA_6);

            //ExportShot(m_db, &m_info, 0, "fourx");
            //for (int i = 1; i < m_info.shots.size(); i++) {
            //    ExportShot(m_db, &m_info, i, fmt::format("fixed_fourx_{:04X}_{:02d}", other_name, static_cast<int>(m_info.shots[i].aid), m_info.shots[i].apx));
            //}

            m_info.tases.clear();
            m_info.tases.emplace_back();
            InitTas(m_db, "/home/matthew/repos/static/data/smb/fm2/nift_454565.fm2", &m_info.tases.back(), niftmin);

            m_info.tases.emplace_back();
            InitTas(m_db, "/home/matthew/repos/static/data/smb/fm2/glitchless.fm2", &m_info.tases.back(), niftmin);

            SyncToArea(&m_info, smb::AreaID::CASTLE_AREA_6);

            {
                int shotn = 14;
                int offset = 100;
                auto& shot = m_info.shots[shotn];
                shot.apx -= offset;
                ExportShot(m_db, &m_info, shotn, fmt::format("454565_vs_glitchless_{:04X}_{:02d}", static_cast<int>(shot.aid), shot.apx));
                shot.apx += offset;
            }
            {
                int shotn = 19;
                auto& shot = m_info.shots[shotn];
                ExportShot(m_db, &m_info, shotn, fmt::format("454565_vs_glitchless_{:04X}_{:02d}", static_cast<int>(shot.aid), shot.apx));
            }
        }
    }
    ImGui::End();
    return true;
}

SMBTasCompareTases::SMBTasCompareTases(smb::SMBDatabase* db, TasCompareInfo* info)
    : m_db(db)
    , m_info(info)
{
}

void SMBTasCompareTases::OnFrame()
{
    if(ImGui::Begin("tases")) {
        ImGui::InputInt("start_frame", &m_info->start_frame);
        ImGui::InputInt("end_frame", &m_info->end_frame);
        rgmui::SliderIntExt("current_frame", &m_info->current_frame, m_info->start_frame, m_info->end_frame);

        if (ImGui::Button("load tas")) {
            std::string tas_path;
            if (nfdext::FileOpenDialog(&tas_path)) {
                m_info->tases.emplace_back();
                if (!InitTas(m_db, tas_path, &m_info->tases.back(), m_info->end_frame)) {
                    m_info->tases.pop_back();
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("load nift")) {
            int niftmin = 19000;
            m_info->tases.clear();
            m_info->tases.emplace_back();
            InitTas(m_db, "/home/matthew/repos/static/data/smb/fm2/tas_454265.fm2", &m_info->tases.back(), niftmin);
            m_info->tases.emplace_back();
            InitTas(m_db, "/home/matthew/repos/static/data/smb/fm2/nift_454565.fm2", &m_info->tases.back(), niftmin);
            m_info->tases.emplace_back();
            InitTas(m_db, "/home/matthew/repos/static/data/smb/fm2/nift_454631.fm2", &m_info->tases.back(), niftmin);
            m_info->tases.emplace_back();
            InitTas(m_db, "/home/matthew/repos/static/data/smb/fm2/nift_454798.fm2", &m_info->tases.back(), niftmin);

            SyncToArea(m_info, smb::AreaID::CASTLE_AREA_6);
        }


        size_t id = 0;
        for (auto & tas : m_info->tases) {
            ImGui::PushID(id++);
            rgmui::TextFmt("path: {} num_frames: {}", tas.path, tas.outputs.size());
            ImGui::InputInt("start: ", &tas.start);
            ImGui::PopID();
        }
    }
    ImGui::End();
}


SMBTasCompareIndividual::SMBTasCompareIndividual(smb::SMBDatabase* db, TasCompareInfo* info)
    : m_db(db)
    , m_info(info)
    , m_individual_index(0)
{
}

void SMBTasCompareIndividual::OnFrame()
{
    if (ImGui::Begin("individual")) {
        rgmui::SliderIntExt("individual index", &m_individual_index, 0, static_cast<int>(m_info->tases.size()) - 1);

        if (m_individual_index >= 0 && m_individual_index < static_cast<int>(m_info->tases.size())) {
            const auto& tas = m_info->tases[m_individual_index];
            int mf = static_cast<int>(tas.frames.size());

            int actual_frame = m_info->current_frame + tas.start + 1;

            if (actual_frame >= 0 && actual_frame < mf) {
                auto m = opencvext::ConstructPaletteImage(tas.frames[actual_frame].data(),
                        nes::FRAME_WIDTH, nes::FRAME_HEIGHT, nes::DefaultPaletteRGB().data());
                rgmui::Mat("frame", m);
            }
        }

    }
    ImGui::End();
}

SMBTasCompareCombined::SMBTasCompareCombined(smb::SMBDatabase* db, TasCompareInfo* info)
    : m_db(db)
    , m_info(info)
{
}

void SMBTasCompareCombined::OnFrame()
{
    if (ImGui::Begin("shots")) {
        if (ImGui::Button("add shot")) {
            m_info->shots.emplace_back();
            m_info->shots.back().aid = smb::AreaID::GROUND_AREA_6;
            m_info->shots.back().apx = 0;
            m_info->shots.back().width = 512;
            m_info->shots.back().follow_tas_index = -1;
        }
    }
    ImGui::End();

    int shot_index = 0;
    for (auto & shot : m_info->shots) {
        if (ImGui::Begin(fmt::format("shot {}", shot_index).c_str())) {
            sta::smbui::AreaIDCombo("area id", &shot.aid);
            rgmui::SliderIntExt("apx", &shot.apx, 0, 10000);
            ImGui::InputInt("width", &shot.width);
            ImGui::InputInt("follow_tas_index", &shot.follow_tas_index);

            auto m = RenderShot(m_db, m_info, shot_index, m_info->current_frame);
            cv::resize(m, m, {}, 2, 2, cv::INTER_NEAREST);
            rgmui::Mat("shot", m);

            if (ImGui::Button("export")) {
                ExportShot(m_db, m_info, shot_index, fmt::format("shot_{}", shot_index));
            }
        }
        ImGui::End();
        shot_index++;
    }
}
