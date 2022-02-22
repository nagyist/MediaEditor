#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <application.h>
#include <imgui_helper.h>
#include <ImGuiFileDialog.h>
#if IMGUI_VULKAN_SHADER
#include <ImVulkanShader.h>
#endif
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include <chrono>
#include "MultiTrackVideoReader.h"
#include "FFUtils.h"
#include "Logger.h"

using namespace std;
using namespace Logger;
using Clock = chrono::steady_clock;

static MultiTrackVideoReader* g_mtVidReader = nullptr;
const int c_videoOutputWidth = 960;
const int c_videoOutputHeight = 540;
const MediaInfo::Ratio c_videoFrameRate = { 25, 1 };

static double g_playStartPos = 0.f;
static Clock::time_point g_playStartTp;
static bool g_isPlay = false;
static bool g_playForward = true;

static ImTextureID g_imageTid;
static ImVec2 g_imageDisplaySize = { 640, 360 };

const string c_imguiIniPath = "ms_test.ini";
const string c_bookmarkPath = "bookmark.ini";

// Application Framework Functions
void Application_GetWindowProperties(ApplicationWindowProperty& property)
{
    property.name = "MultiTrackAudioReaderTest";
    property.viewport = false;
    property.docking = false;
    property.auto_merge = false;
    //property.power_save = false;
    property.width = 1280;
    property.height = 720;
}

void Application_SetupContext(ImGuiContext* ctx)
{
}

void Application_Initialize(void** handle)
{
    GetDefaultLogger()
        ->SetShowLevels(DEBUG);
    GetMultiTrackVideoReaderLogger()
        ->SetShowLevels(DEBUG);

#ifdef USE_BOOKMARK
	// load bookmarks
	ifstream docFile(c_bookmarkPath, ios::in);
	if (docFile.is_open())
	{
		stringstream strStream;
		strStream << docFile.rdbuf(); //read the file
		ImGuiFileDialog::Instance()->DeserializeBookmarks(strStream.str());
		docFile.close();
	}
#endif

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = c_imguiIniPath.c_str();

    g_mtVidReader = CreateMultiTrackVideoReader();
    g_mtVidReader->Configure(c_videoOutputWidth, c_videoOutputHeight, c_videoFrameRate);
    g_mtVidReader->Start();
}

void Application_Finalize(void** handle)
{
    ReleaseMultiTrackVideoReader(&g_mtVidReader);

#ifdef USE_BOOKMARK
	// save bookmarks
	ofstream configFileWriter(c_bookmarkPath, ios::out);
	if (!configFileWriter.bad())
	{
		configFileWriter << ImGuiFileDialog::Instance()->SerializeBookmarks();
		configFileWriter.close();
	}
#endif
}

static uint32_t s_addClipOptSelId = 0;
static double s_addClipTimeLineOffset = 0;
static double s_addClipStartOffset = 0;
static double s_addClipEndOffset = 0;
static uint32_t s_remTrackOptSelId = 0;
static uint32_t s_movClipTrackSelId = 0;
static uint32_t s_movClipSelId = 0;
static double s_changeClipTimeLineOffset = 0;
static double s_changeClipStartOffset = 0;
static double s_changeClipEndOffset = 0;

bool Application_Frame(void * handle, bool app_will_quit)
{
    bool app_done = false;
    auto& io = ImGui::GetIO();

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    if (ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize))
    {
        if (ImGui::Button((string(ICON_IGFD_FOLDER_OPEN)+" Open file").c_str()))
        {
            const char *filters = "视频文件(*.mp4 *.mov *.mkv *.webm *.avi){.mp4,.mov,.mkv,.webm,.avi,.MP4,.MOV,.MKV,WEBM,.AVI},.*";
            ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " 打开视频文件", filters, "/mnt/data2/video/hd/", 1, nullptr, ImGuiFileDialogFlags_ShowBookmark);
        }

        ImGui::SameLine(0, 20);

        vector<string> trackNames;
        for (uint32_t i = 0; i < g_mtVidReader->TrackCount(); i++)
        {
            ostringstream oss;
            oss << "track#" << i+1;
            trackNames.push_back(oss.str());
        }

        vector<string> addClipOpts(trackNames);
        addClipOpts.push_back("new track");
        if (s_addClipOptSelId >= addClipOpts.size())
            s_addClipOptSelId = addClipOpts.size()-1;
        ImGui::PushItemWidth(100);
        ImGui::TextUnformatted("AddClipOptions");
        ImGui::SameLine();
        if (ImGui::BeginCombo("##AddClipOptions", addClipOpts[s_addClipOptSelId].c_str()))
        {
            for (uint32_t i = 0; i < addClipOpts.size(); i++)
            {
                string& item = addClipOpts[i];
                const bool isSelected = s_addClipOptSelId == i;
                if (ImGui::Selectable(item.c_str(), isSelected))
                    s_addClipOptSelId = i;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine(0, 20);
        ImGui::TextUnformatted("TimeLineOffset");
        ImGui::SameLine();
        ImGui::InputDouble("##TimeLineOffset", &s_addClipTimeLineOffset);
        ImGui::SameLine(0, 20);
        ImGui::TextUnformatted("ClipStartOffset");
        ImGui::SameLine();
        ImGui::InputDouble("##ClipStartOffset", &s_addClipStartOffset);
        ImGui::SameLine(0, 20);
        ImGui::TextUnformatted("ClipEndOffset");
        ImGui::SameLine();
        ImGui::InputDouble("##ClipEndOffset", &s_addClipEndOffset);
        ImGui::PopItemWidth();

        ImGui::Spacing();

        vector<string> selectTrackOpts(trackNames);
        if (selectTrackOpts.empty())
            selectTrackOpts.push_back("<No track>");
        bool noTrack = trackNames.empty();

        ImGui::PushItemWidth(100);
        if (ImGui::BeginCombo("##RemTrackOptions", selectTrackOpts[s_remTrackOptSelId].c_str()))
        {
            for (uint32_t i = 0; i < selectTrackOpts.size(); i++)
            {
                string& item = selectTrackOpts[i];
                const bool isSelected = s_remTrackOptSelId == i;
                if (ImGui::Selectable(item.c_str(), isSelected))
                    s_remTrackOptSelId = i;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::SameLine(0, 20);

        if (noTrack)
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        if (ImGui::Button("Remove Track"))
        {
            g_mtVidReader->RemoveTrack(s_remTrackOptSelId);
            s_remTrackOptSelId = 0;
        }
        if (noTrack)
            ImGui::PopItemFlag();

        ImGui::Spacing();

        ImGui::PushItemWidth(100);
        if (ImGui::BeginCombo("##MovClipSelTrackOptions", selectTrackOpts[s_movClipTrackSelId].c_str()))
        {
            for (uint32_t i = 0; i < selectTrackOpts.size(); i++)
            {
                string& item = selectTrackOpts[i];
                const bool isSelected = s_movClipTrackSelId == i;
                if (ImGui::Selectable(item.c_str(), isSelected))
                    s_movClipTrackSelId = i;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine(0, 10);

        vector<string> clipNames;
        if (!noTrack)
        {
            VideoTrackHolder hTrack = g_mtVidReader->GetTrack(s_movClipTrackSelId);
            auto clipIter = hTrack->ClipListBegin();
            while (clipIter != hTrack->ClipListEnd())
            {
                ostringstream oss;
                oss << "Clip#" << (*clipIter)->Id();
                clipNames.push_back(oss.str());
                clipIter++;
            }
        }
        bool noClip = false;
        vector<string> clipSelOpts(clipNames);
        if (clipSelOpts.empty())
        {
            clipSelOpts.push_back("<no clip>");
            noClip = true;
        }
        if (s_movClipSelId >= clipSelOpts.size())
            s_movClipSelId = clipSelOpts.size()-1;
        if (ImGui::BeginCombo("##MovClipSelClipOptions", clipSelOpts[s_movClipSelId].c_str()))
        {
            VideoTrackHolder hTrack = g_mtVidReader->GetTrack(s_movClipTrackSelId);
            auto clipIter = hTrack->ClipListBegin();
            for (uint32_t i = 0; i < clipSelOpts.size(); i++)
            {
                string& item = clipSelOpts[i];
                const bool isSelected = s_movClipSelId == i;
                if (ImGui::Selectable(item.c_str(), isSelected))
                    s_movClipSelId = i;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
                clipIter++;
            }
            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();
        ImGui::SameLine(0, 20);

        if (noClip)
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        if (ImGui::Button("Remove Clip"))
        {
            VideoTrackHolder hTrack = g_mtVidReader->GetTrack(s_movClipTrackSelId);
            hTrack->RemoveClipByIndex(s_movClipSelId);
            s_movClipSelId = 0;
        }
        if (noClip)
            ImGui::PopItemFlag();

        ImGui::SameLine(0, 20);

        ImGui::PushItemWidth(100);
        ImGui::TextUnformatted("tloff");
        ImGui::SameLine();
        ImGui::InputDouble("##tloff", &s_changeClipTimeLineOffset);
        ImGui::SameLine(0, 10);
        ImGui::TextUnformatted("off0");
        ImGui::SameLine();
        ImGui::InputDouble("##off0", &s_changeClipStartOffset);
        ImGui::SameLine(0, 10);
        ImGui::TextUnformatted("off1");
        ImGui::SameLine();
        ImGui::InputDouble("##off1", &s_changeClipEndOffset);
        ImGui::SameLine(0, 10);
        ImGui::PopItemWidth();

        if (noClip)
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        if (ImGui::Button("Change Clip"))
        {
            VideoTrackHolder hTrack = g_mtVidReader->GetTrack(s_movClipTrackSelId);
            VideoClipHolder hClip = hTrack->GetClipByIndex(s_movClipSelId);
            hTrack->ChangeClip(hClip->Id(), s_changeClipTimeLineOffset, s_changeClipStartOffset, s_changeClipEndOffset);
        }
        if (noClip)
            ImGui::PopItemFlag();

        ImGui::Spacing();

        ImGui::TextUnformatted("Video Tracks:");
        uint32_t audTrackIdx = 1;
        for (auto track = g_mtVidReader->TrackListBegin(); track != g_mtVidReader->TrackListEnd(); track++)
        {
            ostringstream oss;
            oss << "Track#" << audTrackIdx++ << ": [";
            for (auto clip = (*track)->ClipListBegin(); clip != (*track)->ClipListEnd();)
            {
                oss << "Clip#" << (*clip)->Id() << ":{'tlOff':" << (*clip)->TimeLineOffset()
                    << ", 'off0':" << (*clip)->StartOffset() << ", 'off1':" << (*clip)->EndOffset()
                    << ", 'dur':" << (*clip)->ClipDuration() << "}";
                clip++;
                if (clip != (*track)->ClipListEnd())
                    oss << ", ";
            }
            oss << "].";
            ImGui::TextUnformatted(oss.str().c_str());
        }

        ImGui::Spacing();

        float playPos;
        float mediaDur = (float)g_mtVidReader->Duration();

        double elapsedTime = chrono::duration_cast<chrono::duration<double>>((Clock::now()-g_playStartTp)).count();
        playPos = g_isPlay ? (g_playForward ? g_playStartPos+elapsedTime : g_playStartPos-elapsedTime) : g_playStartPos;
        if (playPos < 0) playPos = 0;
        if (playPos > mediaDur) playPos = mediaDur;

        ImGui::ImMat vmat;
        if (g_mtVidReader->ReadVideoFrame(playPos, vmat))
        {
            string imgTag = TimestampToString(vmat.time_stamp);
            bool imgValid = true;
            if (vmat.empty())
            {
                imgValid = false;
                imgTag += "(loading)";
            }
            if (imgValid &&
                ((vmat.color_format != IM_CF_RGBA && vmat.color_format != IM_CF_ABGR) ||
                vmat.type != IM_DT_INT8 ||
                (vmat.device != IM_DD_CPU && vmat.device != IM_DD_VULKAN)))
            {
                Log(Error) << "WRONG snapshot format!" << endl;
                imgValid = false;
                imgTag += "(bad format)";
            }
            if (imgValid)
            {
                ImGui::ImMatToTexture(vmat, g_imageTid);
                if (g_imageTid) ImGui::Image(g_imageTid, g_imageDisplaySize);
            }
            else
            {
                ImGui::Dummy(g_imageDisplaySize);
            }
            ImGui::TextUnformatted(imgTag.c_str());
        }

        string playBtnLabel = g_isPlay ? "Pause" : "Play ";
        if (ImGui::Button(playBtnLabel.c_str()))
        {
            g_isPlay = !g_isPlay;
            if (g_isPlay)
                g_playStartTp = Clock::now();
            else
                g_playStartPos = playPos;
        }

        ImGui::SameLine();

        string dirBtnLabel = g_playForward ? "Backword" : "Forward";
        if (ImGui::Button(dirBtnLabel.c_str()))
        {
            bool notForward = !g_playForward;
            g_mtVidReader->SetDirection(notForward);
            g_playForward = notForward;
            g_playStartPos = playPos;
            g_playStartTp = Clock::now();
        }

        ImGui::Spacing();

        ostringstream oss;
        oss << "Video pos: " << TimestampToString(playPos);
        string audTag = oss.str();
        ImGui::TextUnformatted(audTag.c_str());

        ImGui::End();
    }

    // open file dialog
    ImVec2 modal_center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImVec2 maxSize = ImVec2((float)io.DisplaySize.x, (float)io.DisplaySize.y);
	ImVec2 minSize = maxSize * 0.5f;
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
	{
        if (ImGuiFileDialog::Instance()->IsOk())
		{
            string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            if (s_addClipOptSelId == g_mtVidReader->TrackCount())
            {
                if (!g_mtVidReader->AddTrack())
                {
                    Log(Error) << "FAILED to 'AddTrack'! Message is '" << g_mtVidReader->GetError() << "'." << endl;
                }
            }
            VideoTrackHolder hTrack = g_mtVidReader->GetTrack(s_addClipOptSelId);
            hTrack->AddNewClip(filePathName, s_addClipTimeLineOffset, s_addClipStartOffset, s_addClipEndOffset);
            s_addClipOptSelId = g_mtVidReader->TrackCount();
            s_addClipTimeLineOffset = 0;
            s_addClipStartOffset = 0;
            s_addClipEndOffset = 0;
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (!io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape), false))
    {
        app_done = true;
    }
    if (app_will_quit)
    {
        app_done = true;
    }

    return app_done;
}