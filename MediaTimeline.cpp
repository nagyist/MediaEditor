#include "MediaTimeline.h"
#include <imgui_helper.h>
#include <cmath>
#include <sstream>
#include <iomanip>

static std::string MillisecToString(int64_t millisec, int show_millisec = 0)
{
    std::ostringstream oss;
    if (millisec < 0)
    {
        oss << "-";
        millisec = -millisec;
    }
    uint64_t t = (uint64_t) millisec;
    uint32_t milli = (uint32_t)(t%1000); t /= 1000;
    uint32_t sec = (uint32_t)(t%60); t /= 60;
    uint32_t min = (uint32_t)(t%60); t /= 60;
    uint32_t hour = (uint32_t)t;
    if (hour > 0)
    {
        oss << std::setfill('0') << std::setw(2) << hour << ':'
            << std::setw(2) << min << ':'
            << std::setw(2) << sec;
        if (show_millisec == 3)
            oss << '.' << std::setw(3) << milli;
        else if (show_millisec == 2)
            oss << '.' << std::setw(2) << milli / 10;
        else if (show_millisec == 1)
            oss << '.' << std::setw(1) << milli / 100;
    }
    else
    {
        oss << std::setfill('0') << std::setw(2) << min << ':'
            << std::setw(2) << sec;
        if (show_millisec == 3)
            oss << '.' << std::setw(3) << milli;
        else if (show_millisec == 2)
            oss << '.' << std::setw(2) << milli / 10;
        else if (show_millisec == 1)
            oss << '.' << std::setw(1) << milli / 100;
    }
    return oss.str();
}

static bool ExpendButton(ImDrawList *draw_list, ImVec2 pos, bool expand = true)
{
    ImGuiIO &io = ImGui::GetIO();
    ImRect delRect(pos, ImVec2(pos.x + 16, pos.y + 16));
    bool overDel = delRect.Contains(io.MousePos);
    int delColor = IM_COL32_WHITE;
    float midy = pos.y + 16 / 2 - 0.5f;
    float midx = pos.x + 16 / 2 - 0.5f;
    draw_list->AddRect(delRect.Min, delRect.Max, delColor, 4);
    draw_list->AddLine(ImVec2(delRect.Min.x + 3, midy), ImVec2(delRect.Max.x - 4, midy), delColor, 2);
    if (expand) draw_list->AddLine(ImVec2(midx, delRect.Min.y + 3), ImVec2(midx, delRect.Max.y - 4), delColor, 2);
    return overDel;
}

static bool TimelineButton(ImDrawList *draw_list, const char * label, ImVec2 pos, ImVec2 size, std::string tooltips = "", ImVec4 hover_color = ImVec4(0.5f, 0.5f, 0.75f, 1.0f))
{
    ImGuiIO &io = ImGui::GetIO();
    ImRect rect(pos, pos + size);
    bool overButton = rect.Contains(io.MousePos);
    if (overButton)
        draw_list->AddRectFilled(rect.Min, rect.Max, ImGui::GetColorU32(hover_color), 2);
    ImVec4 color = ImVec4(1.f, 1.f, 1.f, 1.f);
    ImGui::SetWindowFontScale(0.75);
    draw_list->AddText(pos, ImGui::GetColorU32(color), label);
    ImGui::SetWindowFontScale(1.0);
    if (overButton && !tooltips.empty())
    {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(tooltips.c_str());
        ImGui::EndTooltip();
    }
    return overButton;
}

static void RenderMouseCursor(const char* mouse_cursor, ImVec2 offset = ImVec2(0 ,0), float base_scale = 1.0, int rotate = 0, ImU32 col_fill = IM_COL32_WHITE, ImU32 col_border = IM_COL32_BLACK, ImU32 col_shadow = IM_COL32(0, 0, 0, 48))
{
    ImGuiViewportP* viewport = (ImGuiViewportP*)ImGui::GetWindowViewport();
    ImDrawList* draw_list = ImGui::GetForegroundDrawList(viewport);
    ImGuiIO& io = ImGui::GetIO();
    const float FontSize = draw_list->_Data->FontSize;
    ImVec2 size(FontSize, FontSize);
    const ImVec2 pos = io.MousePos - offset;
    const float scale = base_scale * viewport->DpiScale;
    if (!viewport->GetMainRect().Overlaps(ImRect(pos, pos + ImVec2(size.x + 2, size.y + 2) * scale)))
        return;

    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    int rotation_start_index = draw_list->VtxBuffer.Size;
    draw_list->AddText(pos + ImVec2(-1, -1), col_border, mouse_cursor);
    draw_list->AddText(pos + ImVec2(1, 1), col_shadow, mouse_cursor);
    draw_list->AddText(pos, col_fill, mouse_cursor);
    if (rotate != 0)
    {
        float rad = M_PI / 180 * (90 - rotate);
        ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX); // bounds
        auto& buf = draw_list->VtxBuffer;
        float s = sin(rad), c = cos(rad);
        for (int i = rotation_start_index; i < buf.Size; i++)
		    l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);
        ImVec2 center = ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2);
	    center = ImRotate(center, s, c) - center;
        
        for (int i = rotation_start_index; i < buf.Size; i++)
		    buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
    }
}

static void alignTime(int64_t& time, MediaInfo::Ratio rate)
{
    if (rate.den && rate.num)
    {
        int64_t frame_index = (int64_t)floor((double)time * (double)rate.num / (double)rate.den / 1000.0);
        time = frame_index * 1000 * rate.den / rate.num;
    }
}

namespace MediaTimeline
{
/***********************************************************************************************************
 * MediaItem Struct Member Functions
 ***********************************************************************************************************/
MediaItem::MediaItem(const std::string& name, const std::string& path, MEDIA_TYPE type)
{
    mID = ImGui::get_current_time_usec(); // sample using system time stamp for Media ID
    mName = name;
    mPath = path;
    mMediaType = type;
    mMediaOverview = CreateMediaOverview();
    if (!path.empty() && mMediaOverview)
    {
        mMediaOverview->SetSnapshotResizeFactor(0.1, 0.1);
        mMediaOverview->Open(path, 50);
    }
    if (mMediaOverview && mMediaOverview->IsOpened())
    {
        mStart = 0;
        if (mMediaOverview->HasVideo() && type != MEDIA_PICTURE)
        {
            mMediaOverview->GetMediaParser()->EnableParseInfo(MediaParser::VIDEO_SEEK_POINTS);
            mEnd = mMediaOverview->GetVideoDuration();
        }
        else if (mMediaOverview->HasAudio())
        {
            mEnd = mMediaOverview->GetMediaInfo()->duration * 1000;
        }
        else
        {
            // image or text?
            mEnd = 1000;
        }
    }
}

MediaItem::~MediaItem()
{
    ReleaseMediaOverview(&mMediaOverview);
    mMediaOverview = nullptr;
    for (auto thumb : mMediaThumbnail)
    {
        ImGui::ImDestroyTexture(thumb); 
        thumb = nullptr;
    }
}
void MediaItem::UpdateThumbnail()
{
    if (mMediaOverview && mMediaOverview->IsOpened())
    {
        auto count = mMediaOverview->GetSnapshotCount();
        if (mMediaThumbnail.size() >= count)
            return;
        std::vector<ImGui::ImMat> snapshots;
        if (mMediaOverview->GetSnapshots(snapshots))
        {
            for (int i = 0; i < snapshots.size(); i++)
            {
                if (i >= mMediaThumbnail.size() && !snapshots[i].empty())
                {
                    ImTextureID thumb = nullptr;
                    ImMatToTexture(snapshots[i], thumb);
                    mMediaThumbnail.push_back(thumb);
                }
            }
        }
    }
}
} //namespace MediaTimeline

namespace MediaTimeline
{
/***********************************************************************************************************
 * Clip Struct Member Functions
 ***********************************************************************************************************/
Clip::Clip(int64_t start, int64_t end, int64_t id, MediaOverview * overview, void * handle)
{
    mID = ImGui::get_current_time_usec(); // sample using system time stamp for Clip ID
    mMediaID = id;
    mStart = start; 
    mEnd = end;
    mHandle = handle;
    mOverview = overview;
}

Clip::~Clip()
{
}

void Clip::Load(Clip * clip, const imgui_json::value& value)
{
    if (value.contains("ID"))
    {
        auto& val = value["ID"];
        if (val.is_number()) clip->mID = val.get<imgui_json::number>();
    }
    if (value.contains("MediaID"))
    {
        auto& val = value["MediaID"];
        if (val.is_number()) clip->mMediaID = val.get<imgui_json::number>();
    }
    if (value.contains("GroupID"))
    {
        auto& val = value["GroupID"];
        if (val.is_number()) clip->mGroupID = val.get<imgui_json::number>();
    }
    if (value.contains("Start"))
    {
        auto& val = value["Start"];
        if (val.is_number()) clip->mStart = val.get<imgui_json::number>();
    }
    if (value.contains("End"))
    {
        auto& val = value["End"];
        if (val.is_number()) clip->mEnd = val.get<imgui_json::number>();
    }
    if (value.contains("StartOffset"))
    {
        auto& val = value["StartOffset"];
        if (val.is_number()) clip->mStartOffset = val.get<imgui_json::number>();
    }
    if (value.contains("EndOffset"))
    {
        auto& val = value["EndOffset"];
        if (val.is_number()) clip->mEndOffset = val.get<imgui_json::number>();
    }
    if (value.contains("Selected"))
    {
        auto& val = value["Selected"];
        if (val.is_boolean()) clip->bSelected = val.get<imgui_json::boolean>();
    }
    if (value.contains("Editting"))
    {
        auto& val = value["Editting"];
        if (val.is_boolean()) clip->bEditting = val.get<imgui_json::boolean>();
    }
    // load filter bp
    if (value.contains("FilterBP"))
    {
        auto& val = value["FilterBP"];
        if (val.is_object()) clip->mFilterBP = val;
    }
}

void Clip::Save(imgui_json::value& value)
{
    // save clip global info
    value["ID"] = imgui_json::number(mID);
    value["MediaID"] = imgui_json::number(mMediaID);
    value["GroupID"] = imgui_json::number(mGroupID);
    value["Type"] = imgui_json::number(mType);
    value["Path"] = mPath;
    value["Name"] = mName;
    value["Start"] = imgui_json::number(mStart);
    value["End"] = imgui_json::number(mEnd);
    value["StartOffset"] = imgui_json::number(mStartOffset);
    value["EndOffset"] = imgui_json::number(mEndOffset);
    value["Selected"] = imgui_json::boolean(bSelected);
    value["Editting"] = imgui_json::boolean(bEditting);

    // save clip filter bp
    if (mFilterBP.is_object())
    {
        value["FilterBP"] = mFilterBP;
    }
}

int64_t Clip::Cropping(int64_t diff, int type)
{
    int64_t new_diff = 0;
    TimeLine * timeline = (TimeLine *)mHandle;
    if (!timeline)
        return new_diff;
    float frame_duration = (timeline->mFrameRate.den > 0 && timeline->mFrameRate.num > 0) ? timeline->mFrameRate.den * 1000.0 / timeline->mFrameRate.num : 40;
    if (type == 0)
    {
        // cropping start
        if (mType == MEDIA_VIDEO || mType == MEDIA_AUDIO)
        {
            // audio video stream have length limit
            if (mStart + diff < mEnd - ceil(frame_duration) && mStart + diff >= timeline->mStart)
            {
                if (mStartOffset + diff >= 0)
                {
                    new_diff = diff;
                    mStart += diff;
                    mStartOffset += diff;
                }
                else if (abs(mStartOffset + diff) <= abs(diff))
                {
                    new_diff = diff + abs(mStartOffset + diff);
                    mStart += new_diff;
                    mStartOffset += new_diff;
                }
            }
            else if (mEnd - mStart - ceil(frame_duration) < diff)
            {
                new_diff = mEnd - mStart - ceil(frame_duration);
                mStart += new_diff;
                mStartOffset += new_diff;
            }
        }
        else
        {
            // others have not length limit
            if (mStart + diff < mEnd - ceil(frame_duration) && mStart + diff >= timeline->mStart)
            {
                new_diff = diff;
                mStart += diff;
            }
            else if (mEnd - mStart - ceil(frame_duration) < diff)
            {
                new_diff = mEnd - mStart - ceil(frame_duration);
                mStart += new_diff;
            }
        }
    }
    else
    {
        // cropping end
        if (mType == MEDIA_VIDEO || mType == MEDIA_AUDIO)
        {
            // audio video stream have length limit
            if (mEnd + diff > mStart + ceil(frame_duration))
            {
                if (mEndOffset - diff >= 0)
                {
                    new_diff = diff;
                    mEnd += new_diff;
                    mEndOffset -= new_diff;
                }
                else if (abs(mEndOffset - diff) <= abs(diff))
                {
                    new_diff = diff - abs(mEndOffset - diff);
                    mEnd += new_diff;
                    mEndOffset -= new_diff;
                }
            }
            else if (mEnd - mStart - ceil(frame_duration) < abs(diff))
            {
                new_diff = mStart - mEnd + ceil(frame_duration);
                mEnd += new_diff;
                mEndOffset -= new_diff;
            }
        }
        else
        {
            // others have not length limit
            if (mEnd + diff > mStart + ceil(frame_duration))
            {
                new_diff = diff;
                mEnd += new_diff;
            }
            else if (mEnd - mStart - ceil(frame_duration) < abs(diff))
            {
                new_diff = mStart - mEnd + ceil(frame_duration);
                mEnd += new_diff;
            }
        }
    }
    return new_diff;
}

void Clip::Cutting(int64_t pos)
{
    TimeLine * timeline = (TimeLine *)mHandle;
    if (!timeline)
        return;
    auto track = timeline->FindTrackByClipID(mID);
    if (!track)
        return;
    
    // calculate new pos
    int64_t adj_end = pos;
    int64_t adj_end_offset = mEndOffset + (mEnd - pos);
    int64_t new_start = pos;
    int64_t new_start_offset = mStartOffset + (pos - mStart);
    // create new clip base on current clip
    Clip * new_clip = nullptr;
    switch (mType)
    {
        case MEDIA_VIDEO:
        {
            auto new_video_clip = new VideoClip(mStart, mEnd, mMediaID, mName, mOverview, timeline);
            new_clip = new_video_clip;
        }
        break;
        case MEDIA_AUDIO:
        {
            auto new_audio_clip = new AudioClip(mStart, mEnd, mMediaID, mName, mOverview, timeline);
            new_clip = new_audio_clip;
        }
        break;
        case MEDIA_PICTURE:
        {
            auto new_image_clip = new ImageClip(mStart, mEnd, mMediaID, mName, mOverview, timeline);
            new_clip = new_image_clip;
            new_start_offset = 0;
            adj_end_offset = 0;
        }
        break;
        case MEDIA_TEXT:
        {
            auto new_text_clip = new TextClip(mStart, mEnd, mMediaID, mName, mOverview, timeline);
            new_clip = new_text_clip;
        }
        break;
        default:
        break;
    }

    // insert new clip into track and timeline
    if (new_clip)
    {
        
        new_clip->mStart = new_start;
        new_clip->mStartOffset = new_start_offset;
        new_clip->mEnd = mEnd;
        new_clip->mEndOffset = mEndOffset;
        mEnd = adj_end;
        mEndOffset = adj_end_offset;
        timeline->m_Clips.push_back(new_clip);
        track->InsertClip(new_clip, pos);
        timeline->AddClipIntoGroup(new_clip, mGroupID);
        timeline->Updata();
    }
}

int64_t Clip::Moving(int64_t diff, int mouse_track)
{
    int64_t index = -1;
    TimeLine * timeline = (TimeLine *)mHandle;
    if (!timeline)
        return index;
    auto track = timeline->FindTrackByClipID(mID);
    if (!track)
        return index;
    
    int track_index = timeline->FindTrackIndexByClipID(mID);
    int64_t length = mEnd - mStart;
    int64_t start = timeline->mStart;
    int64_t end = -1;
    int64_t new_diff = -1;
    /*
    auto prov_clip = track->FindPrevClip(mID);
    auto next_clip = track->FindNextClip(mID);
    
    if (prov_clip)
    {
        start = prov_clip->mEnd;
    }
    else 
    {
        start = timeline->mStart;
    }
    if (next_clip)
    {
        end = next_clip->mStart;
    }
    else
    {
        end = -1;
    }
    */

    if (mStart + diff < start)
    {
        new_diff = start - mStart;
        mStart = start;
        mEnd = mStart + length;
    }
    else if (end != -1 && mEnd + diff > end)
    {
        new_diff = end - mEnd;
        mStart = end - length;
        mEnd = mStart + length;
    }
    else
    {
        new_diff = diff;
        mStart += diff;
        mEnd = mStart + length;
    }
    
    // check clip is cross track
    if (mouse_track == -2)
    {
        index = timeline->NewTrack(mType, track->mExpanded);
        timeline->MovingClip(mID, track_index, index);
        // move to empty area, need create new track?
    }
    else if (mouse_track >= 0 && mouse_track != track_index)
    {
        auto media_type = timeline->m_Tracks[mouse_track]->mType;
        if (mType == media_type)
        {
            // clip move into other same type track
            timeline->MovingClip(mID, track_index, mouse_track);
            index = mouse_track;
        }
    }

    if (timeline->bSelectLinked)
    {
        for (auto &clip : timeline->m_Clips)
        {
            if (clip->bSelected && clip->mID != mID)
            {
                int64_t clip_length = clip->mEnd - clip->mStart;
                clip->mStart += new_diff;
                clip->mEnd = clip->mStart + clip_length;
            }
        }
    }
    timeline->Updata();
    //ImGui::BeginTooltip();
    //ImGui::Text("%d", mouse_track);
    //ImGui::EndTooltip();
    return index;
}

bool Clip::isLinkedWith(Clip * clip)
{
    TimeLine * timeline = (TimeLine *)mHandle;
    if (!timeline)
        return false;
    if (mGroupID != -1 && mGroupID == clip->mGroupID)
        return true;

    return false;
}

// VideoClip Struct Member Functions
VideoClip::VideoClip(int64_t start, int64_t end, int64_t id, std::string name, MediaOverview * overview, void* handle)
    : Clip(start, end, id, overview, handle)
{
    if (handle && overview)
    {
        mSnapshot = CreateMediaSnapshot();
        if (!mSnapshot)
            return;
        mType = MEDIA_VIDEO;
        mName = name;
        MediaInfo::InfoHolder info = mOverview->GetMediaInfo();
        const MediaInfo::VideoStream* video_stream = mOverview->GetVideoStream();
        if (!info || !video_stream)
        {
            ReleaseMediaSnapshot(&mSnapshot);
            return;
        }
        mPath = info->url;
        MediaParserHolder holder = mOverview->GetMediaParser();
        mSnapshot->Open(holder);
        if (mSnapshot->IsOpened())
        {
            mClipFrameRate = video_stream->avgFrameRate;
            double window_size = 1.0f;
            mSnapshot->SetCacheFactor(1.0);
            mSnapshot->SetSnapshotResizeFactor(0.1, 0.1);
            mSnapshot->ConfigSnapWindow(window_size, 1);
        }
    }
}

VideoClip::~VideoClip()
{
    ReleaseMediaSnapshot(&mSnapshot);
    for (auto& snap : mVideoSnapshots)
    {
        if (snap.texture) { ImGui::ImDestroyTexture(snap.texture); snap.texture = nullptr; }
    }
    mVideoSnapshots.clear();
}

void VideoClip::UpdateSnapshot()
{
}
    
void VideoClip::Seek()
{
}

void VideoClip::Step(bool forward, int64_t step)
{
}

Clip* VideoClip::Load(const imgui_json::value& value, void * handle)
{
    TimeLine * timeline = (TimeLine *)handle;
    if (!timeline || timeline->media_items.size() <= 0)
        return nullptr;

    int64_t id = -1;
    MediaItem * item = nullptr;
    if (value.contains("MediaID"))
    {
        auto& val = value["MediaID"];
        if (val.is_number()) id = val.get<imgui_json::number>();
    }
    if (id != -1)
    {
        item = timeline->FindMediaItemByID(id);
    }
    if (item)
    {
        // media is in bank
        VideoClip * new_clip = new VideoClip(item->mStart, item->mEnd, item->mID, item->mName, item->mMediaOverview, handle);
        if (new_clip)
        {
            Clip::Load(new_clip, value);
            // load video info
            if (value.contains("FrameRateNum"))
            {
                auto& val = value["FrameRateNum"];
                if (val.is_number()) new_clip->mClipFrameRate.num = val.get<imgui_json::number>();
            }
            if (value.contains("FrameRateDen"))
            {
                auto& val = value["FrameRateDen"];
                if (val.is_number()) new_clip->mClipFrameRate.den = val.get<imgui_json::number>();
            }
            return new_clip;
        }
    }
    else
    {
        // media isn't in bank we need create new media item first ?
    }
    return nullptr;
}

void VideoClip::Save(imgui_json::value& value)
{
    Clip::Save(value);
    // save video clip info
    value["FrameRateNum"] = imgui_json::number(mClipFrameRate.num);
    value["FrameRateDen"] = imgui_json::number(mClipFrameRate.den);
}

// AudioClip Struct Member Functions
AudioClip::AudioClip(int64_t start, int64_t end, int64_t id, std::string name, MediaOverview * overview, void* handle)
    : Clip(start, end, id, overview, handle)
{
    if (handle && overview)
    {
        mType = MEDIA_AUDIO;
        mName = name;
        MediaInfo::InfoHolder info = mOverview->GetMediaInfo();
        const MediaInfo::AudioStream* audio_stream = mOverview->GetAudioStream();
        if (!info || !audio_stream)
        {
            return;
        }
        mPath = info->url;
        MediaParserHolder holder = mOverview->GetMediaParser();
        // TODO::Dicky
    }
}

AudioClip::~AudioClip()
{
}

void AudioClip::UpdateSnapshot()
{
}

void AudioClip::Seek()
{
}

void AudioClip::Step(bool forward, int64_t step)
{
}

bool AudioClip::GetFrame(std::pair<ImGui::ImMat, ImGui::ImMat>& in_out_frame)
{
    return false;
}

Clip * AudioClip::Load(const imgui_json::value& value, void * handle)
{
    TimeLine * timeline = (TimeLine *)handle;
    if (!timeline || timeline->media_items.size() <= 0)
        return nullptr;
    
    int64_t id = -1;
    MediaItem * item = nullptr;
    if (value.contains("MediaID"))
    {
        auto& val = value["MediaID"];
        if (val.is_number()) id = val.get<imgui_json::number>();
    }
    if (id != -1)
    {
        item = timeline->FindMediaItemByID(id);
    }
    if (item)
    {
        AudioClip * new_clip = new AudioClip(item->mStart, item->mEnd, item->mID, item->mName, item->mMediaOverview, handle);
        if (new_clip)
        {
            Clip::Load(new_clip, value);
            // load audio info
            if (value.contains("Channels"))
            {
                auto& val = value["Channels"];
                if (val.is_number()) new_clip->mAudioChannels = val.get<imgui_json::number>();
            }
            if (value.contains("SampleRate"))
            {
                auto& val = value["SampleRate"];
                if (val.is_number()) new_clip->mAudioSampleRate = val.get<imgui_json::number>();
            }
            if (value.contains("Format"))
            {
                auto& val = value["Format"];
                if (val.is_number()) new_clip->mAudioFormat = (AudioRender::PcmFormat)val.get<imgui_json::number>();
            }
            return new_clip;
        }
    }
    else
    {
        // media isn't in bank we need create new media item first ?
    }
    return nullptr;
}

void AudioClip::Save(imgui_json::value& value)
{
    Clip::Save(value);
    // save audio clip info
    value["Channels"] = imgui_json::number(mAudioChannels);
    value["SampleRate"] = imgui_json::number(mAudioSampleRate);
    value["Format"] = imgui_json::number(mAudioFormat);
}

// ImageClip Struct Member Functions
ImageClip::ImageClip(int64_t start, int64_t end, int64_t id, std::string name, MediaOverview * overview, void* handle)
    : Clip(start, end, id, overview, handle)
{
    if (handle && overview)
    {
        mType = MEDIA_PICTURE;
        mName = name;
        MediaInfo::InfoHolder info = mOverview->GetMediaInfo();
        if (!info)
        {
            return;
        }
        mPath = info->url;
        MediaParserHolder holder = mOverview->GetMediaParser();
        // TODO::Dicky
    }
}

ImageClip::~ImageClip()
{
}

void ImageClip::UpdateSnapshot()
{
}

void ImageClip::Seek()
{
}

void ImageClip::Step(bool forward, int64_t step)
{
}

bool ImageClip::GetFrame(std::pair<ImGui::ImMat, ImGui::ImMat>& in_out_frame)
{
    return false;
}

Clip * ImageClip::Load(const imgui_json::value& value, void * handle)
{
    TimeLine * timeline = (TimeLine *)handle;
    if (!timeline || timeline->media_items.size() <= 0)
        return nullptr;

    int64_t id = -1;
    MediaItem * item = nullptr;
    if (value.contains("MediaID"))
    {
        auto& val = value["MediaID"];
        if (val.is_number()) id = val.get<imgui_json::number>();
    }
    if (id != -1)
    {
        item = timeline->FindMediaItemByID(id);
    }

    if (item)
    {
        ImageClip * new_clip = new ImageClip(item->mStart, item->mEnd, item->mID, item->mName, item->mMediaOverview, handle);
        if (new_clip)
        {
            Clip::Load(new_clip, value);
            // load image info
            return new_clip;
        }
    }
    else
    {
        // media isn't in bank we need create new media item first ?
    }
    return nullptr;
}

void ImageClip::Save(imgui_json::value& value)
{
    Clip::Save(value);
    // save image clip info
}

// TextClip Struct Member Functions
TextClip::TextClip(int64_t start, int64_t end, int64_t id, std::string name, MediaOverview * overview, void* handle)
    : Clip(start, end, id, overview, handle)
{
    if (handle && overview)
    {
        mType = MEDIA_TEXT;
        mName = name;
        MediaInfo::InfoHolder info = mOverview->GetMediaInfo();
        if (!info)
        {
            return;
        }
        mPath = info->url;
        MediaParserHolder holder = mOverview->GetMediaParser();
        // TODO::Dicky
    }
}

TextClip::~TextClip()
{
}

void TextClip::UpdateSnapshot()
{
}

void TextClip::Seek()
{
}

void TextClip::Step(bool forward, int64_t step)
{
}

bool TextClip::GetFrame(std::pair<ImGui::ImMat, ImGui::ImMat>& in_out_frame)
{
    return false;
}

Clip * TextClip::Load(const imgui_json::value& value, void * handle)
{
    TimeLine * timeline = (TimeLine *)handle;
    if (!timeline || timeline->media_items.size() <= 0)
        return nullptr;
    
    int64_t id = -1;
    MediaItem * item = nullptr;
    if (value.contains("MediaID"))
    {
        auto& val = value["MediaID"];
        if (val.is_number()) id = val.get<imgui_json::number>();
    }
    if (id != -1)
    {
        item = timeline->FindMediaItemByID(id);
    }

    if (item)
    {
        TextClip * new_clip = new TextClip(item->mStart, item->mEnd, item->mID, item->mName, item->mMediaOverview, handle);
        if (new_clip)
        {
            Clip::Load(new_clip, value);
            // load text info
            return new_clip;
        }
    }
    else
    {
        // media isn't in bank we need create new media item first ?
    }
    return nullptr;
}

void TextClip::Save(imgui_json::value& value)
{
    Clip::Save(value);
    // save Text clip info
}
} //namespace MediaTimeline/Clip

namespace MediaTimeline
{
Overlap::Overlap(int64_t start, int64_t end, int64_t clip_first, int64_t clip_second, void* handle)
{
    mID = ImGui::get_current_time_usec(); // sample using system time stamp for Overlap ID
    mStart = start;
    mEnd = end;
    m_Clip.first = clip_first;
    m_Clip.second = clip_second;
    mHandle = handle;
}

Overlap::~Overlap()
{

}

bool Overlap::IsOverlapValid()
{
    TimeLine * timeline = (TimeLine *)mHandle;
    if (!timeline)
        return false;
    
    auto clip_start = timeline->FindClipByID(m_Clip.first);
    auto clip_end = timeline->FindClipByID(m_Clip.second);
    if (!clip_start || !clip_end)
        return false;
    auto track_start = timeline->FindTrackByClipID(clip_start->mID);
    auto track_end = timeline->FindTrackByClipID(clip_end->mID);
    if (!track_start || !track_end || track_start->mID != track_end->mID)
        return false;
    
    if (clip_start->mEnd <= clip_end->mStart ||
        clip_end->mEnd <= clip_start->mStart)
        return false;
    
    return true;
}

void Overlap::Update(int64_t start, int64_t start_clip_id, int64_t end, int64_t end_clip_id)
{
    m_Clip.first = start_clip_id;
    m_Clip.second = end_clip_id;
    mStart = start;
    mEnd = end;
}

Overlap* Overlap::Load(const imgui_json::value& value, void * handle)
{
    TimeLine * timeline = (TimeLine *)handle;
    if (!timeline)
        return nullptr;
    int64_t start = 0;
    int64_t end = 0;
    int64_t first = -1;
    int64_t second = -1;
    if (value.contains("Start"))
    {
        auto& val = value["Start"];
        if (val.is_number()) start = val.get<imgui_json::number>();
    }
    if (value.contains("End"))
    {
        auto& val = value["End"];
        if (val.is_number()) end = val.get<imgui_json::number>();
    }
    if (value.contains("Clip_First"))
    {
        auto& val = value["Clip_First"];
        if (val.is_number()) first = val.get<imgui_json::number>();
    }
    if (value.contains("Clip_Second"))
    {
        auto& val = value["Clip_Second"];
        if (val.is_number()) second = val.get<imgui_json::number>();
    }

    Overlap * new_overlap = new Overlap(start, end, first, second, handle);
    if (new_overlap)
    {
        if (value.contains("ID"))
        {
            auto& val = value["ID"];
            if (val.is_number()) new_overlap->mID = val.get<imgui_json::number>();
        }
        if (value.contains("Editting"))
        {
            auto& val = value["Editting"];
            if (val.is_boolean()) new_overlap->bEditting = val.get<imgui_json::boolean>();
        }
        // load fusion bp
        if (value.contains("FusionBP"))
        {
            auto& val = value["FusionBP"];
            if (val.is_object()) new_overlap->mFusionBP = val;
        }
    }
    return new_overlap;
}

void Overlap::Save(imgui_json::value& value)
{
    // save overlap global info
    value["ID"] = imgui_json::number(mID);
    value["Start"] = imgui_json::number(mStart);
    value["End"] = imgui_json::number(mEnd);
    value["Clip_First"] = imgui_json::number(m_Clip.first);
    value["Clip_Second"] = imgui_json::number(m_Clip.second);
    value["Editting"] = imgui_json::boolean(bEditting);

    // save overlap fusion bp
    if (mFusionBP.is_object())
    {
        value["FusionBP"] = mFusionBP;
    }
}

}// namespace MediaTimeline

namespace MediaTimeline
{
/***********************************************************************************************************
 * MediaTrack Struct Member Functions
 ***********************************************************************************************************/
MediaTrack::MediaTrack(std::string name, MEDIA_TYPE type, void * handle)
    : m_Handle(handle),
      mType(type)
{
    mID = ImGui::get_current_time_usec(); // sample using system time stamp for Track ID
    if (name.empty())
    {
        TimeLine * timeline = (TimeLine *)m_Handle;
        if (timeline)
        {
            auto media_count = timeline->GetTrackCount(type);
            media_count ++;
            switch (type)
            {
                case MEDIA_VIDEO:
                    mName = "V:";
                    mTrackHeight = DEFAULT_VIDEO_TRACK_HEIGHT;
                break;
                case MEDIA_AUDIO:
                    mName = "A:";
                    mTrackHeight = DEFAULT_AUDIO_TRACK_HEIGHT;
                break;
                case MEDIA_PICTURE:
                    mName = "P:";
                    mTrackHeight = DEFAULT_IMAGE_TRACK_HEIGHT;
                break;
                case MEDIA_TEXT:
                    mName = "T:";
                    mTrackHeight = DEFAULT_TEXT_TRACK_HEIGHT;
                break;
                default:
                    mName = "U:";
                    mTrackHeight = DEFAULT_TRACK_HEIGHT;
                break;
            }
            mName += std::to_string(media_count);
        }
    }
    else
    {
        mName = name;
    }
}

MediaTrack::~MediaTrack()
{
    TimeLine * timeline = (TimeLine *)m_Handle;
    if (!timeline)
        return;

    // remove overlaps from timeline
    for (auto overlap : m_Overlaps)
    {
        timeline->DeleteOverlap(overlap->mID);
    }

    // remove clips from timeline
    for (auto clip : m_Clips)
    {
        timeline->DeleteClip(clip->mID);
    }

    // remove linked track info
    auto linked_track = timeline->FindTrackByID(mLinkedTrack);
    if (linked_track)
    {
        linked_track->mLinkedTrack = -1;
    }
}

bool MediaTrack::DrawTrackControlBar(ImDrawList *draw_list, ImRect rc)
{
    bool need_update = false;
    ImGuiIO &io = ImGui::GetIO();
    if (mExpanded) draw_list->AddText(rc.Min + ImVec2(4, 0), IM_COL32_WHITE, mName.c_str());
    ImVec2 button_size = ImVec2(12, 12);
    int button_count = 0;
    {
        bool ret = TimelineButton(draw_list, mLocked ? ICON_LOCKED : ICON_UNLOCK, ImVec2(rc.Min.x + button_size.x * button_count * 1.5 + 6, rc.Max.y - button_size.y - 2), button_size, mLocked ? "unlock" : "lock");
        if (ret && io.MouseReleased[0])
            mLocked = !mLocked;
        button_count ++;
    }
    if (mType == MEDIA_AUDIO)
    {
        bool ret = TimelineButton(draw_list, mView ? ICON_SPEAKER_MUTE : ICON_SPEAKER, ImVec2(rc.Min.x + button_size.x * button_count * 1.5 + 6, rc.Max.y - button_size.y - 2), button_size, mView ? "voice" : "mute");
        if (ret && io.MouseReleased[0])
            mView = !mView;
        button_count ++;
    }
    else
    {
        bool ret = TimelineButton(draw_list, mView ? ICON_VIEW : ICON_VIEW_DISABLE, ImVec2(rc.Min.x + button_size.x * button_count * 1.5 + 6, rc.Max.y - button_size.y - 2), button_size, mView ? "hidden" : "view");
        if (ret && io.MouseReleased[0])
        {
            mView = !mView;
            need_update = true;
        }
        button_count ++;
    }
    return need_update;
}

void MediaTrack::Update()
{
    TimeLine * timeline = (TimeLine *)m_Handle;
    if (!timeline)
        return;
    // sort m_Clips by clip start time
    std::sort(m_Clips.begin(), m_Clips.end(), CompareClip);
    
    // check all overlaps
    for (auto iter = m_Overlaps.begin(); iter != m_Overlaps.end();)
    {
        if (!(*iter)->IsOverlapValid())
        {
            int64_t id = (*iter)->mID;
            iter = m_Overlaps.erase(iter);
            timeline->DeleteOverlap(id);
        }
        else
            ++iter;
    }

    // check is there have new overlap area
    for (auto iter = m_Clips.begin(); iter != m_Clips.end(); iter++)
    {
        for (auto next = iter + 1; next != m_Clips.end(); next++)
        {
            if ((*iter)->mEnd >= (*next)->mStart)
            {
                // it is a overlap area
                int64_t start = std::max((*next)->mStart, (*iter)->mStart);
                int64_t end = std::min((*iter)->mEnd, (*next)->mEnd);
                // check it is in exist overlaps
                auto overlap = FindExistOverlap((*iter)->mID, (*next)->mID);
                if (overlap)
                    overlap->Update(start, (*iter)->mID, end, (*next)->mID);
                else
                    CreateOverlap(start, (*iter)->mID, end, (*next)->mID);
            }
        }
    }
}

void MediaTrack::CreateOverlap(int64_t start, int64_t start_clip_id, int64_t end, int64_t end_clip_id)
{
    TimeLine * timeline = (TimeLine *)m_Handle;
    if (!timeline)
        return;

    Overlap * new_overlap = new Overlap(start, end, start_clip_id, end_clip_id, timeline);
    timeline->m_Overlaps.push_back(new_overlap);
    m_Overlaps.push_back(new_overlap);
}

Overlap * MediaTrack::FindExistOverlap(int64_t start_clip_id, int64_t end_clip_id)
{
    Overlap * found_overlap = nullptr;
    for (auto overlap : m_Overlaps)
    {
        if ((overlap->m_Clip.first == start_clip_id && overlap->m_Clip.second == end_clip_id) ||
            (overlap->m_Clip.first == end_clip_id && overlap->m_Clip.second == start_clip_id))
        {
            found_overlap = overlap;
            break;
        }

    }
    return found_overlap;
}

void MediaTrack::DeleteClip(int64_t id)
{
    TimeLine * timeline = (TimeLine *)m_Handle;
    if (!timeline)
        return;
    auto iter = std::find_if(m_Clips.begin(), m_Clips.end(), [id](const Clip* clip)
    {
        return clip->mID == id;
    });
    if (iter != m_Clips.end())
    {
        m_Clips.erase(iter);
    }
    Update();
}

void MediaTrack::PushBackClip(Clip * clip)
{
    if (m_Clips.size() > 0)
    {
        int64_t length = clip->mEnd - clip->mStart;
        auto last_clip = m_Clips.end() - 1;
        clip->mStart = (*last_clip)->mEnd;
        clip->mEnd = clip->mStart + length;
    }
    m_Clips.push_back(clip);
}

void MediaTrack::InsertClip(Clip * clip, int64_t pos)
{
    TimeLine * timeline = (TimeLine *)m_Handle;
    if (!timeline || !clip)
        return;
    auto iter = std::find_if(m_Clips.begin(), m_Clips.end(), [clip](const Clip * _clip)
    {
        return _clip->mID == clip->mID;
    });
    if (iter == m_Clips.end())
    {
        int64_t length = clip->mEnd - clip->mStart;
        clip->mStart = pos;
        clip->mEnd = clip->mStart + length;
        m_Clips.push_back(clip);
    }
/*
    int64_t length = clip->mEnd - clip->mStart;
    // check insert pos and range is valid
    int64_t pos_token_end = -1;
    int64_t next_start = -1;
    if (pos == -1)
    {
        // we insert clip end of track
        PushBackClip(clip);
        return;
    }

    for (auto _clip : m_Clips)
    {
        if (_clip && _clip->mStart <= pos && _clip->mEnd >= pos)
        {
            pos_token_end = _clip->mEnd;
            next_start = timeline->NextClipStart(_clip);
            break;
        }
    }
    if (pos_token_end != -1)
    {
        // pos already has clip
        if (next_start != -1)
        {
            int64_t space = next_start - pos_token_end;
            if (length <= space)
            {
                // we insert clip after pos_token_end
                clip->mStart = pos_token_end;
                clip->mEnd = clip->mStart + length;
            }
            else
            {
                // we insert clip end of track
                PushBackClip(clip);
                return;
            }
        }
        else
        {
            // we insert clip after pos_token_end because current clip is end clip
            clip->mStart = pos_token_end;
            clip->mEnd = clip->mStart + length;
        }
    }
    else
    {
         // pos is empty
        next_start = timeline->NextClipStart(pos);
        if (next_start != -1)
        {
            int64_t space = next_start - pos;
            if (length <= space)
            {
                // we insert clip at pos
                clip->mStart = pos;
                clip->mEnd = clip->mStart + length;
            }
            else
            {
                // we insert clip end of track
                PushBackClip(clip);
                return;
            }
        }
        else
        {
            // we insert clip after pos because current pos is end of track
            clip->mStart = pos;
            clip->mEnd = clip->mStart + length;
        }
    }

    m_Clips.push_back(clip);
*/
    Update();
}

Clip * MediaTrack::FindPrevClip(int64_t id)
{
    Clip * found_clip = nullptr;
    TimeLine * timeline = (TimeLine *)m_Handle;
    if (!timeline)
        return found_clip;
    auto current_clip = timeline->FindClipByID(id);
    if (!current_clip)
        return found_clip;
    auto iter = std::find_if(m_Clips.begin(), m_Clips.end(), [id](const Clip* ss) {
        return ss->mID == id;
    });
    if (iter == m_Clips.begin() || iter == m_Clips.end())
        return found_clip;
    
    found_clip = *(iter - 1);
    return found_clip;
}

Clip * MediaTrack::FindNextClip(int64_t id)
{
    Clip * found_clip = nullptr;
    TimeLine * timeline = (TimeLine *)m_Handle;
    if (!timeline)
        return found_clip;
    auto current_clip = timeline->FindClipByID(id);
    if (!current_clip)
        return found_clip;
    auto iter = std::find_if(m_Clips.begin(), m_Clips.end(), [id](const Clip* ss) {
        return ss->mID == id;
    });

    if (iter == m_Clips.end() || iter == m_Clips.end() - 1)
        return found_clip;

    found_clip = *(iter + 1);
    return found_clip;
}

Clip * MediaTrack::FindClips(int64_t time, int& count)
{
    Clip * ret_clip = nullptr;
    int selected_count = 0;
    std::vector<Clip *> clips;
    std::vector<Clip *> select_clips;
    for (auto clip : m_Clips)
    {
        if (clip->mStart <= time && clip->mEnd >= time)
        {
            clips.push_back(clip);
            if (clip->bSelected)
                select_clips.push_back(clip);
        }
    }
    for (auto clip : select_clips)
    {
        if (!ret_clip || ret_clip->mEnd - ret_clip->mStart > clip->mEnd - clip->mStart)
            ret_clip = clip;
    }
    if (!ret_clip)
    {
        for (auto clip : clips)
        {
            if (!ret_clip || ret_clip->mEnd - ret_clip->mStart > clip->mEnd - clip->mStart)
                ret_clip = clip;
        }
    }
    count = clips.size();
    return ret_clip;
}

void MediaTrack::SelectClip(Clip * clip, bool appand)
{
    TimeLine * timeline = (TimeLine *)m_Handle;
    if (!timeline || !clip)
        return;

    bool selected = true;
    if (appand && clip->bSelected)
    {
        selected = false;
    }
    
    for (auto _clip : timeline->m_Clips)
    {
        if (_clip->mID != clip->mID)
        {
            if (timeline->bSelectLinked && _clip->isLinkedWith(clip))
            {
                _clip->bSelected = selected;
            }
            else if (!appand)
            {
                _clip->bSelected = !selected;
            }
        }
    }
    clip->bSelected = selected;
}

void MediaTrack::EdittingClip(Clip * clip)
{
    TimeLine * timeline = (TimeLine *)m_Handle;
    if (!timeline || !clip)
        return;
    for (auto _clip : timeline->m_Clips)
    {
        if (_clip->mID != clip->mID)
        {
            _clip->bEditting = false;
        }
    }
    clip->bEditting = true;
}

void MediaTrack::EdittingOverlap(Overlap * overlap)
{
    TimeLine * timeline = (TimeLine *)m_Handle;
    if (!timeline || !overlap)
        return;
    for (auto _overlap : timeline->m_Overlaps)
    {
        if (_overlap->mID != overlap->mID)
        {
            _overlap->bEditting = false;
        }
    }
    overlap->bEditting = true;
}

MediaTrack* MediaTrack::Load(const imgui_json::value& value, void * handle)
{
    MEDIA_TYPE type = MEDIA_UNKNOWN;
    std::string name;
    TimeLine * timeline = (TimeLine *)handle;
    if (!timeline)
        return nullptr;
    
    if (value.contains("Type"))
    {
        auto& val = value["Type"];
        if (val.is_number()) type = (MEDIA_TYPE)val.get<imgui_json::number>();
    }
    if (value.contains("Name"))
    {
        auto& val = value["Name"];
        if (val.is_string()) name = val.get<imgui_json::string>();
    }
    MediaTrack * new_track = new MediaTrack(name, type, handle);
    if (new_track)
    {
        if (value.contains("ID"))
        {
            auto& val = value["ID"];
            if (val.is_number()) new_track->mID = val.get<imgui_json::number>();
        }
        if (value.contains("Expanded"))
        {
            auto& val = value["Expanded"];
            if (val.is_boolean()) new_track->mExpanded = val.get<imgui_json::boolean>();
        }
        if (value.contains("View"))
        {
            auto& val = value["View"];
            if (val.is_boolean()) new_track->mView = val.get<imgui_json::boolean>();
        }
        if (value.contains("Locked"))
        {
            auto& val = value["Locked"];
            if (val.is_boolean()) new_track->mLocked = val.get<imgui_json::boolean>();
        }
        if (value.contains("Selected"))
        {
            auto& val = value["Selected"];
            if (val.is_boolean()) new_track->mSelected = val.get<imgui_json::boolean>();
        }
        if (value.contains("Linked"))
        {
            auto& val = value["Linked"];
            if (val.is_number()) new_track->mLinkedTrack = val.get<imgui_json::number>();
        }
        if (value.contains("ViewHeight"))
        {
            auto& val = value["ViewHeight"];
            if (val.is_number()) new_track->mTrackHeight = val.get<imgui_json::number>();
        }

        // load and check clip into track
        const imgui_json::array* clipIDArray = nullptr;
        if (BluePrint::GetPtrTo(value, "ClipIDS", clipIDArray))
        {
            for (auto& id_val : *clipIDArray)
            {
                int64_t clip_id = id_val.get<imgui_json::number>();
                Clip * clip = timeline->FindClipByID(clip_id);
                if (clip)
                    new_track->m_Clips.push_back(clip);
            }
        }

        // load and check overlap into track
        const imgui_json::array* overlapIDArray = nullptr;
        if (BluePrint::GetPtrTo(value, "OverlapIDS", overlapIDArray))
        {
            for (auto& id_val : *overlapIDArray)
            {
                int64_t overlap_id = id_val.get<imgui_json::number>();
                Overlap * overlap = timeline->FindOverlapByID(overlap_id);
                if (overlap)
                    new_track->m_Overlaps.push_back(overlap);
            }
        }
    }
    return new_track;
}

void MediaTrack::Save(imgui_json::value& value)
{
    // save track info
    value["ID"] = imgui_json::number(mID);
    value["Type"] = imgui_json::number(mType);
    value["Name"] = mName;
    value["Expanded"] = imgui_json::boolean(mExpanded);
    value["View"] = imgui_json::boolean(mView);
    value["Locked"] = imgui_json::boolean(mLocked);
    value["Selected"] = imgui_json::boolean(mSelected);
    value["Linked"] = imgui_json::number(mLinkedTrack);
    value["ViewHeight"] = imgui_json::number(mTrackHeight);

    // save clip ids
    imgui_json::value clips;
    for (auto clip : m_Clips)
    {
        imgui_json::value clip_id_value = imgui_json::number(clip->mID);
        clips.push_back(clip_id_value);
    }
    if (m_Clips.size() > 0) value["ClipIDS"] = clips;

    // save overlap ids
    imgui_json::value overlaps;
    for (auto overlap : m_Overlaps)
    {
        imgui_json::value overlap_id_value = imgui_json::number(overlap->mID);
        overlaps.push_back(overlap_id_value);
    }
    if (m_Overlaps.size() > 0) value["OverlapIDS"] = overlaps;
}

} // namespace MediaTimeline/MediaTrack

namespace MediaTimeline
{
/***********************************************************************************************************
 * TimeLine Struct Member Functions
 ***********************************************************************************************************/
void ClipGroup::Load(const imgui_json::value& value)
{
    if (value.contains("ID"))
    {
        auto& val = value["ID"];
        if (val.is_number()) mID = val.get<imgui_json::number>();
    }
    const imgui_json::array* clipIDArray = nullptr;
    if (BluePrint::GetPtrTo(value, "ClipIDS", clipIDArray))
    {
        for (auto& id_val : *clipIDArray)
        {
            int64_t clip_id = id_val.get<imgui_json::number>();
            m_Grouped_Clips.push_back(clip_id);
        }
    }
}

void ClipGroup::Save(imgui_json::value& value)
{
    if (m_Grouped_Clips.size() > 0)
    {
        value["ID"] = imgui_json::number(mID);
        imgui_json::value clips;
        for (auto clip : m_Grouped_Clips)
        {
            imgui_json::value clip_id_value = imgui_json::number(clip);
            clips.push_back(clip_id_value);
        }
        value["ClipIDS"] = clips;
    }
}

int TimeLine::OnBluePrintChange(int type, std::string name, void* handle)
{
    int ret = BluePrint::BP_CBR_Nothing;
    if (!handle)
        return BluePrint::BP_CBR_Unknown;
    TimeLine * timeline = (TimeLine *)handle;
    if (name.compare("VideoFilter") == 0)
    {
        if (type == BluePrint::BP_CB_Link ||
            type == BluePrint::BP_CB_Unlink ||
            type == BluePrint::BP_CB_NODE_DELETED)
        {
            timeline->mVideoFilterNeedUpdate = true;
            ret = BluePrint::BP_CBR_AutoLink;
        }
        else if (type == BluePrint::BP_CB_PARAM_CHANGED ||
                type == BluePrint::BP_CB_SETTING_CHANGED)
        {
            timeline->mVideoFilterNeedUpdate = true;
        }
    }
    if (name.compare("VideoFusion") == 0)
    {
    }

    return ret;
}

TimeLine::TimeLine()
    : mStart(0), mEnd(0)
{
    /*
    mPCMStream = new SequencerPcmStream(this);
    mAudioRender = CreateAudioRender();
    if (mAudioRender)
    {
        mAudioRender->OpenDevice(mAudioSampleRate, mAudioChannels, mAudioFormat, mPCMStream);
    }
    */

    mVideoFilterBluePrint = new BluePrint::BluePrintUI();
    if (mVideoFilterBluePrint)
    {
        BluePrint::BluePrintCallbackFunctions callbacks;
        callbacks.BluePrintOnChanged = OnBluePrintChange;
        mVideoFilterBluePrint->Initialize();
        mVideoFilterBluePrint->SetCallbacks(callbacks, this);
    }

    mAudioFilterBluePrint = new BluePrint::BluePrintUI();
    if (mAudioFilterBluePrint)
    {
        BluePrint::BluePrintCallbackFunctions callbacks;
        callbacks.BluePrintOnChanged = OnBluePrintChange;
        mAudioFilterBluePrint->Initialize();
        mAudioFilterBluePrint->SetCallbacks(callbacks, this);
    }

    mVideoFusionBluePrint = new BluePrint::BluePrintUI();
    if (mVideoFusionBluePrint)
    {
        BluePrint::BluePrintCallbackFunctions callbacks;
        callbacks.BluePrintOnChanged = OnBluePrintChange;
        mVideoFusionBluePrint->Initialize();
        mVideoFusionBluePrint->SetCallbacks(callbacks, this);
    }

    for (int i = 0; i < mAudioChannels; i++)
        mAudioLevel.push_back(0);
}

TimeLine::~TimeLine()
{
    mFrameLock.lock();
    mFrame.clear();
    mFrameLock.unlock();

    if (mMainPreviewTexture) { ImGui::ImDestroyTexture(mMainPreviewTexture); mMainPreviewTexture = nullptr; }
    mAudioLevel.clear();
    if (mVideoFilterBluePrint)
    {
        mVideoFilterBluePrint->Finalize();
        delete mVideoFilterBluePrint;
    }
    if (mAudioFilterBluePrint)
    {
        mAudioFilterBluePrint->Finalize();
        delete mAudioFilterBluePrint;
    }
    if (mVideoFusionBluePrint)
    {
        mVideoFusionBluePrint->Finalize();
        delete mVideoFusionBluePrint;
    }
    for (auto item : media_items) delete item;
    for (auto track : m_Tracks) delete track;
    for (auto clip : m_Clips) delete clip;
}

void TimeLine::AlignTime(int64_t& time)
{
    alignTime(time, mFrameRate);
}

void TimeLine::Updata()
{
    int64_t start_min = INT64_MAX;
    int64_t end_max = INT64_MIN;
    for (auto clip : m_Clips)
    {
        if (clip->mStart < start_min)
            start_min = clip->mStart;
        if (clip->mEnd > end_max)
            end_max = clip->mEnd;
    }
    if (start_min < mStart)
        mStart = ImMax(start_min, (int64_t)0);
    if (end_max > mEnd)
    {
        mEnd = end_max + TIMELINE_OVER_LENGTH;
    }

    // update track
    mTrackLock.lock();
    for (auto track : m_Tracks)
    {
        track->Update();
    }
    mTrackLock.unlock();
}

void TimeLine::Click(int index, int64_t time)
{
    bool click_empty_space = true;
    if (index >= 0 && index < m_Tracks.size())
    {
        auto current_track = m_Tracks[index];
        for (auto clip : current_track->m_Clips)
        {
            if (clip->mStart <= time && clip->mEnd >= time)
            {
                click_empty_space = false;
                break;
            }
        }
    }
    if (click_empty_space)
    {
        // clear selected
        for (auto clip : m_Clips)
        {
            clip->bSelected = false;
        }
    }
}

void TimeLine::DoubleClick(int index, int64_t time)
{
    if (index >= 0 && index < m_Tracks.size())
    {
        bool click_empty_space = true;
        auto current_track = m_Tracks[index];
        for (auto overlap : m_Overlaps)
        {
            if (overlap->mStart <= time && overlap->mEnd >= time)
            {
                click_empty_space = false;
                break;
            }
        }
        if (click_empty_space)
        {
            current_track->mExpanded = !current_track->mExpanded;
        }
    }
}

void TimeLine::SelectTrack(int index)
{
    mTrackLock.lock();
    if (index >= 0 && index < m_Tracks.size())
    {
        auto current_track = m_Tracks[index];
        for (auto track : m_Tracks)
        {
            if (track->mID != current_track->mID)
                track->mSelected = false;
            else
                track->mSelected = true;
        }
    }
    mTrackLock.unlock();
}

void TimeLine::DeleteTrack(int index)
{
    mTrackLock.lock();
    if (index >= 0 && index < m_Tracks.size())
    {
        auto track = m_Tracks[index];
        m_Tracks.erase(m_Tracks.begin() + index);
        delete track;
        if (m_Tracks.size() == 0)
        {
            mStart = mEnd = 0;
            currentTime = firstTime = lastTime = visibleTime = 0;
        }
    }
    mTrackLock.unlock();
}

int TimeLine::NewTrack(MEDIA_TYPE type, bool expand)
{
    auto new_track = new MediaTrack("", type, this);
    new_track->mExpanded = expand;
    mTrackLock.try_lock();
    m_Tracks.push_back(new_track);
    mTrackLock.unlock();
    Updata();
    return m_Tracks.size() - 1;
}

void TimeLine::MovingTrack(int& index, int& dst_index)
{
    mTrackLock.lock();
    auto iter = m_Tracks.begin() + index;
    auto iter_dst = dst_index == -2 ? m_Tracks.end() - 1 : m_Tracks.begin() + dst_index;
    if (dst_index == -2 && iter == m_Tracks.end() - 1)
    {
        mTrackLock.unlock();
        return;
    }
    MediaTrack * tmp = *iter;
    *iter = *iter_dst;
    *iter_dst = tmp;
    index = dst_index;
    mTrackLock.unlock();
}

void TimeLine::MovingClip(int64_t id, int from_track_index, int to_track_index)
{
    if (from_track_index < 0 || to_track_index < 0 ||
        from_track_index >= m_Tracks.size() || to_track_index >= m_Tracks.size())
        return;
    mTrackLock.try_lock();
    auto track = m_Tracks[from_track_index];
    auto dst_track = m_Tracks[to_track_index];
    if (!track || !dst_track)
    {
        mTrackLock.unlock();
        return;
    }
    // remove clip from source track
    for (auto iter = track->m_Clips.begin(); iter != track->m_Clips.end();)
    {
        if ((*iter)->mID == id)
        {
            iter = track->m_Clips.erase(iter);
        }
        else
            ++iter;
    }
    // find clip in timeline
    auto iter = std::find_if(m_Clips.begin(), m_Clips.end(), [id](const Clip* clip) {
        return clip->mID == id;
    });
    if (iter != m_Clips.end())
    {
        auto clip = *iter;
        dst_track->InsertClip(clip, clip->mStart);
    }

    mTrackLock.unlock();
}

void TimeLine::DeleteClip(int64_t id)
{
    auto iter = std::find_if(m_Clips.begin(), m_Clips.end(), [id](const Clip* clip) {
        return clip->mID == id;
    });

    if (iter != m_Clips.end())
    {
        auto clip = *iter;
        // need check overlap and remove ?
        m_Clips.erase(iter);
        DeleteClipFromGroup(clip, clip->mGroupID);
        delete clip;
    }
}

void TimeLine::DeleteOverlap(int64_t id)
{
    for (auto iter = m_Overlaps.begin(); iter != m_Overlaps.end();)
    {
        if ((*iter)->mID == id)
        {
            Overlap * overlap = *iter;
            iter = m_Overlaps.erase(iter);
            delete overlap;
        }
        else
            ++ iter;
    }
}

ImGui::ImMat TimeLine::GetPreviewFrame()
{
    ImGui::ImMat frame;
    return frame;
}

int TimeLine::GetAudioLevel(int channel)
{
    if (channel < mAudioLevel.size())
        return mAudioLevel[channel];
    return 0;
}

int TimeLine::GetSelectedClipCount()
{
    int count = 0;
    for (auto clip : m_Clips)
    {
        if (clip->bSelected) count++;
    }
    return count;
}

void TimeLine::Play(bool play, bool forward)
{

}

void TimeLine::Step(bool forward)
{

}

void TimeLine::Loop(bool loop)
{

}

void TimeLine::ToStart()
{

}

void TimeLine::ToEnd()
{

}

MediaItem* TimeLine::FindMediaItemByName(std::string name)
{
    for (auto media: media_items)
    {
        if (media->mName.compare(name) == 0)
            return media;
    }
    return nullptr;
}

MediaItem* TimeLine::FindMediaItemByID(int64_t id)
{
    for (auto media: media_items)
    {
        if (media->mID == id)
            return media;
    }
    return nullptr;
}

MediaTrack * TimeLine::FindTrackByID(int64_t id)
{
    MediaTrack * track_found = nullptr;
    mTrackLock.try_lock();
    for (auto track : m_Tracks)
    {
        if (track->mID == id)
        {
            track_found = track;
            break;
        }
    }
    mTrackLock.unlock();
    return track_found;
}

MediaTrack * TimeLine::FindTrackByClipID(int64_t id)
{
    MediaTrack * track_found = nullptr;
    mTrackLock.try_lock();
    for (auto track : m_Tracks)
    {
        for (auto clip : track->m_Clips)
        {
            if (clip->mID == id)
            {
                track_found = track;
                break;
            }
        }
        if (track_found)
            break;
    }
    mTrackLock.unlock();
    return track_found;
}

int TimeLine::FindTrackIndexByClipID(int64_t id)
{
    int track_found = -1;
    mTrackLock.try_lock();
    for (int i = 0; i < m_Tracks.size(); i++)
    {
        auto track = m_Tracks[i];
        for (auto clip : track->m_Clips)
        {
            if (clip->mID == id)
            {
                track_found = i;
                break;
            }
        }
        if (track_found != -1)
            break;
    }
    mTrackLock.unlock();
    return track_found;
}

Clip * TimeLine::FindClipByID(int64_t id)
{
    Clip * clip_found = nullptr;
    for (auto clip : m_Clips)
    {
        if (clip->mID == id)
        {
            clip_found = clip;
            break;
        }
    }
    return clip_found;
}

Overlap * TimeLine::FindOverlapByID(int64_t id)
{
    Overlap * overlap_found = nullptr;
    for (auto overlap : m_Overlaps)
    {
        if (overlap->mID == id)
        {
            overlap_found = overlap;
            break;
        }
    }
    return overlap_found;
}

int64_t TimeLine::NextClipStart(Clip * clip)
{
    int64_t next_start = -1;
    if (clip)
    {
        for (auto _clip : m_Clips)
        {
            if (_clip->mStart >= clip->mEnd)
            {
                next_start = _clip->mStart;
                break;
            }
        }
    }

    return next_start;
}

int64_t TimeLine::NextClipStart(int64_t pos)
{
    int64_t next_start = -1;
    for (auto _clip : m_Clips)
    {
        if (_clip->mStart > pos)
        {
            next_start = _clip->mStart;
            break;
        }
    }
    return next_start;
}

int TimeLine::GetTrackCount(MEDIA_TYPE type)
{
    int count = 0;
    mTrackLock.lock();
    for (auto track : m_Tracks)
    {
        if (track->mType == type)
            count ++;
    }
    mTrackLock.unlock();
    return count;
}

int64_t TimeLine::NewGroup(Clip * clip)
{
    if (!clip)
        return -1;
    DeleteClipFromGroup(clip, clip->mGroupID);
    ClipGroup new_group;
    new_group.m_Grouped_Clips.push_back(clip->mID);
    m_Groups.push_back(new_group);
    clip->mGroupID = new_group.mID;
    return clip->mGroupID;
}

void TimeLine::AddClipIntoGroup(Clip * clip, int64_t group_id)
{
    if (!clip || group_id == -1 || clip->mGroupID == group_id)
        return;
    // remove clip if clip is already in some group
    DeleteClipFromGroup(clip, clip->mGroupID);
    for (auto & group : m_Groups)
    {
        if (group_id == group.mID)
        {
            group.m_Grouped_Clips.push_back(clip->mID);
            clip->mGroupID = group_id;
        }
    }
}

void TimeLine::DeleteClipFromGroup(Clip *clip, int64_t group_id)
{
    if (group_id == -1 || !clip)
        return;
    for (auto iter = m_Groups.begin(); iter != m_Groups.end();)
    {
        bool need_erase = false;
        if ((*iter).mID == group_id)
        {
            auto clip_iter = std::find_if((*iter).m_Grouped_Clips.begin(), (*iter).m_Grouped_Clips.end(), [clip](const int64_t& id) 
            {
                return clip->mID == id;
            });
            if (clip_iter != (*iter).m_Grouped_Clips.end())
            {
                (*iter).m_Grouped_Clips.erase(clip_iter);
                if ((*iter).m_Grouped_Clips.size() <= 0)
                    need_erase = true;
            }
        }
        if (need_erase)
        {
            iter = m_Groups.erase(iter);
        }
        else
            ++iter;
    }
    clip->mGroupID = -1;
}

ClipGroup TimeLine::GetGroupByID(int64_t group_id)
{
    if (group_id == -1)
        return {};
    for (auto group : m_Groups)
    {
        if (group.mID == group_id)
            return group;
    }
    return {};
}

void TimeLine::CustomDraw(int index, ImDrawList *draw_list, const ImRect &rc, const ImRect &titleRect, const ImRect &clippingTitleRect, const ImRect &legendRect, const ImRect &clippingRect, const ImRect &legendClippingRect, int64_t viewStartTime, int64_t visibleTime, float pixelWidth, bool need_update, bool enable_select)
{
    // rc: full track length rect
    // titleRect: full track length title rect(same as Compact view rc)
    // clippingTitleRect: current view title area
    // clippingRect: current view window area
    // legendRect: legend area
    // legendClippingRect: legend area
    bool mouse_clicked = false;
    int64_t viewEndTime = viewStartTime + visibleTime;
    ImGuiIO &io = ImGui::GetIO();
    if (index >= m_Tracks.size())
        return;
    MediaTrack *track = m_Tracks[index];
    if (!track)
        return;

    // draw legend
    draw_list->PushClipRect(legendRect.Min, legendRect.Max, true);
    draw_list->AddRect(legendRect.Min, legendRect.Max, COL_DEEP_DARK, 0, 0, 2);
    auto need_seek = track->DrawTrackControlBar(draw_list, legendRect);
    //if (need_seek) Seek();
    draw_list->PopClipRect();
    
    // draw clips
    for (auto clip : track->m_Clips)
    {
        bool draw_clip = false;
        float cursor_start = 0;
        float cursor_end  = 0;
        if (clip->mStart <= viewStartTime && clip->mEnd > viewStartTime && clip->mEnd <= viewEndTime)
        {
            /***********************************************************
             *         ----------------------------------------
             * XXXXXXXX|XXXXXXXXXXXXXXXXXXXXXX|
             *         ----------------------------------------
            ************************************************************/
            cursor_start = clippingRect.Min.x;
            cursor_end = clippingRect.Min.x + (clip->mEnd - viewStartTime) * pixelWidth;
            draw_clip = true;
        }
        else if (clip->mStart >= viewStartTime && clip->mEnd <= viewEndTime)
        {
            /***********************************************************
             *         ----------------------------------------
             *                  |XXXXXXXXXXXXXXXXXXXXXX|
             *         ----------------------------------------
            ************************************************************/
            cursor_start = clippingRect.Min.x + (clip->mStart - viewStartTime) * pixelWidth;
            cursor_end = clippingRect.Min.x + (clip->mEnd - viewStartTime) * pixelWidth;
            draw_clip = true;
        }
        else if (clip->mStart >= viewStartTime && clip->mStart < viewEndTime && clip->mEnd >= viewEndTime)
        {
            /***********************************************************
             *         ----------------------------------------
             *                         |XXXXXXXXXXXXXXXXXXXXXX|XXXXXXXXX
             *         ----------------------------------------
            ************************************************************/
            cursor_start = clippingRect.Min.x + (clip->mStart - viewStartTime) * pixelWidth;
            cursor_end = clippingRect.Max.x;
            draw_clip = true;
        }
        else if (clip->mStart <= viewStartTime && clip->mEnd >= viewEndTime)
        {
            /***********************************************************
             *         ----------------------------------------
             *  XXXXXXX|XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX|XXXXXXXX
             *         ----------------------------------------
            ************************************************************/
            cursor_start = clippingRect.Min.x;
            cursor_end  = clippingRect.Max.x;
            draw_clip = true;
        }
        if (draw_clip && cursor_end > cursor_start)
        {
            // draw title bar
            ImVec2 clip_title_pos_min = ImVec2(cursor_start, clippingTitleRect.Min.y);
            ImVec2 clip_title_pos_max = ImVec2(cursor_end, clippingTitleRect.Max.y);
            if (clip->mGroupID != -1)
                draw_list->AddRectFilled(clip_title_pos_min, clip_title_pos_max, IM_COL32(32,255,32,128));
            else
                draw_list->AddRectFilled(clip_title_pos_min, clip_title_pos_max, IM_COL32(32,128,32,128));
            draw_list->AddRect(clip_title_pos_min, clip_title_pos_max, IM_COL32_BLACK);
            // draw clip status
            draw_list->PushClipRect(clip_title_pos_min, clip_title_pos_max, true);
            draw_list->AddText(clip_title_pos_min, IM_COL32_WHITE, clip->mName.c_str());
            draw_list->PopClipRect();
            ImVec2 clip_pos_min = clip_title_pos_min;
            ImVec2 clip_pos_max = clip_title_pos_max;

            // draw custom view
            if (track->mExpanded)
            {
                draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
                ImVec2 custom_pos_min = ImVec2(cursor_start, clippingRect.Min.y);
                ImVec2 custom_pos_max = ImVec2(cursor_end, clippingRect.Max.y);
                draw_list->AddRect(custom_pos_min, custom_pos_max, IM_COL32_BLACK);
                draw_list->PopClipRect();
                clip_pos_min = custom_pos_min;
                clip_pos_max = custom_pos_max;
            }

            if (clip->bSelected)
            {
                if (clip->bEditting)
                    draw_list->AddRect(clip_pos_min, clip_pos_max, IM_COL32(255,0,255,224), 0, 0, 2.0f);
                else
                    draw_list->AddRect(clip_pos_min, clip_pos_max, IM_COL32(255,0,0,224), 0, 0, 2.0f);
            }
            else if (clip->bEditting)
            {
                draw_list->AddRect(clip_pos_min, clip_pos_max, IM_COL32(0,0,255,224), 0, 0, 2.0f);
            }

            // Clip select
            if (enable_select)
            {
                ImGui::SetCursorScreenPos(clip_title_pos_min);
                auto id_string = clip->mPath + "@" + std::to_string(clip->mStart) + "@" + std::to_string(clip->mID);
                ImGui::BeginChildFrame(ImGui::GetID(("track_clips::" + id_string).c_str()), clip_pos_max - clip_title_pos_min, ImGuiWindowFlags_NoScrollbar);
                ImGui::InvisibleButton(id_string.c_str(), clip_pos_max - clip_title_pos_min);
                if (ImGui::IsItemHovered())
                {
                    if (!mouse_clicked && io.MouseClicked[0] && !io.MouseDoubleClicked[0])
                    {
                        const bool is_shift_key_only = (io.KeyMods == ImGuiKeyModFlags_Shift);
                        bool appand = (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)) && is_shift_key_only;
                        track->SelectClip(clip, appand);
                        SelectTrack(index);
                        mouse_clicked = true;
                    }
                    else if (io.MouseDoubleClicked[0])
                    {
                        track->EdittingClip(clip);
                    }
                }
                ImGui::EndChildFrame();
            }
        }
    }

    // draw overlap
    draw_list->PushClipRect(clippingTitleRect.Min, clippingTitleRect.Max, true);
    for (auto overlap : track->m_Overlaps)
    {
        bool draw_overlap = false;
        float cursor_start = 0;
        float cursor_end  = 0;
        if (overlap->mStart >= viewStartTime && overlap->mEnd < viewEndTime)
        {
            cursor_start = clippingTitleRect.Min.x + (overlap->mStart - viewStartTime) * pixelWidth;
            cursor_end = clippingTitleRect.Min.x + (overlap->mEnd - viewStartTime) * pixelWidth;
            draw_overlap = true;
        }
        else if (overlap->mStart >= viewStartTime && overlap->mStart <= viewEndTime && overlap->mEnd >= viewEndTime)
        {
            cursor_start = clippingTitleRect.Min.x + (overlap->mStart - viewStartTime) * pixelWidth;
            cursor_end = clippingTitleRect.Max.x;
            draw_overlap = true;
        }
        else if (overlap->mStart <= viewStartTime && overlap->mEnd <= viewEndTime)
        {
            cursor_start = clippingTitleRect.Min.x;
            cursor_end = clippingTitleRect.Min.x + (overlap->mEnd - viewStartTime) * pixelWidth;
            draw_overlap = true;
        }
        else if (overlap->mStart <= viewStartTime && overlap->mEnd >= viewEndTime)
        {
            cursor_start = clippingTitleRect.Min.x;
            cursor_end  = clippingTitleRect.Max.x;
            draw_overlap = true;
        }
        if (draw_overlap && cursor_end > cursor_start)
        {
            ImVec2 overlap_pos_min = ImVec2(cursor_start, clippingTitleRect.Min.y);
            ImVec2 overlap_pos_max = ImVec2(cursor_end, clippingTitleRect.Max.y);
            ImRect overlap_rect(overlap_pos_min, overlap_pos_max);
            ImGui::SetCursorScreenPos(overlap_pos_min);
            auto id_string = track->mName + "@" + std::to_string(overlap->mID);
            ImGui::BeginChildFrame(ImGui::GetID(("clip_overlap::" + id_string).c_str()), overlap_pos_max - overlap_pos_min, ImGuiWindowFlags_NoScrollbar);
            ImGui::InvisibleButton(id_string.c_str(), overlap_pos_max - overlap_pos_min);
            if (ImGui::IsItemHovered())
            {
                draw_list->AddRectFilled(overlap_pos_min, overlap_pos_max, IM_COL32(255,255,32,192));
                if (io.MouseDoubleClicked[0])
                {
                    track->EdittingOverlap(overlap);
                }
            }
            else
                draw_list->AddRectFilled(overlap_pos_min, overlap_pos_max, IM_COL32(128,128,32,192));
            
            draw_list->AddLine(overlap_pos_min, overlap_pos_max, IM_COL32(0, 0, 0, 255));
            draw_list->AddLine(ImVec2(overlap_pos_max.x, overlap_pos_min.y), ImVec2(overlap_pos_min.x, overlap_pos_max.y), IM_COL32(0, 0, 0, 255));
            
            if (overlap->bEditting)
            {
                draw_list->AddRect(overlap_pos_min, overlap_pos_max, IM_COL32(255, 0, 255, 255));
            }
            ImGui::EndChildFrame();
        }
    }
    draw_list->PopClipRect();
}

int TimeLine::Load(const imgui_json::value& value)
{
    // load media clip
    const imgui_json::array* mediaClipArray = nullptr;
    if (BluePrint::GetPtrTo(value, "MediaClip", mediaClipArray))
    {
        for (auto& clip : *mediaClipArray)
        {
            MEDIA_TYPE type = MEDIA_UNKNOWN;
            if (clip.contains("Type"))
            {
                auto& val = clip["Type"];
                if (val.is_number()) type = (MEDIA_TYPE)val.get<imgui_json::number>();
            }
            Clip * media_clip = nullptr;
            switch (type)
            {
                case MEDIA_VIDEO: media_clip = VideoClip::Load(clip, this); break;
                case MEDIA_AUDIO: media_clip = AudioClip::Load(clip, this); break;
                case MEDIA_PICTURE: media_clip = ImageClip::Load(clip, this); break;
                case MEDIA_TEXT: media_clip = TextClip::Load(clip, this); break;
                default:
                break;
            }
            if (media_clip)
                m_Clips.push_back(media_clip);
        }
    }

    // load media group
    const imgui_json::array* mediaGroupArray = nullptr;
    if (BluePrint::GetPtrTo(value, "MediaGroup", mediaGroupArray))
    {
        for (auto& group : *mediaGroupArray)
        {
            ClipGroup new_group;
            new_group.Load(group);
            m_Groups.push_back(new_group);
        }
    }

    // load media overlap
    const imgui_json::array* mediaOverlapArray = nullptr;
    if (BluePrint::GetPtrTo(value, "MediaOverlap", mediaOverlapArray))
    {
        for (auto& overlap : *mediaOverlapArray)
        {
            Overlap * new_overlap = Overlap::Load(overlap, this);
            if (new_overlap)
                m_Overlaps.push_back(new_overlap);
        }
    }

    // load media track
    const imgui_json::array* mediaTrackArray = nullptr;
    if (BluePrint::GetPtrTo(value, "MediaTrack", mediaTrackArray))
    {
        for (auto& track : *mediaTrackArray)
        {
            MediaTrack * media_track = MediaTrack::Load(track, this);
            if (media_track)
            {
                m_Tracks.push_back(media_track);
            }
        }
    }

    // load global info
    if (value.contains("Start"))
    {
        auto& val = value["Start"];
        if (val.is_number()) mStart = val.get<imgui_json::number>();
    }
    if (value.contains("End"))
    {
        auto& val = value["End"];
        if (val.is_number()) mEnd = val.get<imgui_json::number>();
    }
    if (value.contains("VideoWidth"))
    {
        auto& val = value["VideoWidth"];
        if (val.is_number()) mWidth = val.get<imgui_json::number>();
    }
    if (value.contains("VideoHeight"))
    {
        auto& val = value["VideoHeight"];
        if (val.is_number()) mHeight = val.get<imgui_json::number>();
    }
    if (value.contains("FrameRateNum"))
    {
        auto& val = value["FrameRateNum"];
        if (val.is_number()) mFrameRate.num = val.get<imgui_json::number>();
    }
    if (value.contains("FrameRateDen"))
    {
        auto& val = value["FrameRateDen"];
        if (val.is_number()) mFrameRate.den = val.get<imgui_json::number>();
    }
    if (value.contains("AudioChannels"))
    {
        auto& val = value["AudioChannels"];
        if (val.is_number()) mAudioChannels = val.get<imgui_json::number>();
    }
    if (value.contains("AudioSampleRate"))
    {
        auto& val = value["AudioSampleRate"];
        if (val.is_number()) mAudioSampleRate = val.get<imgui_json::number>();
    }
    if (value.contains("AudioFormat"))
    {
        auto& val = value["AudioFormat"];
        if (val.is_number()) mAudioFormat = (AudioRender::PcmFormat)val.get<imgui_json::number>();
    }
    if (value.contains("msPixelWidth"))
    {
        auto& val = value["msPixelWidth"];
        if (val.is_number()) msPixelWidthTarget = val.get<imgui_json::number>();
    }
    if (value.contains("FirstTime"))
    {
        auto& val = value["FirstTime"];
        if (val.is_number()) firstTime = val.get<imgui_json::number>();
    }
    if (value.contains("CurrentTime"))
    {
        auto& val = value["CurrentTime"];
        if (val.is_number()) currentTime = val.get<imgui_json::number>();
    }
    if (value.contains("Forward"))
    {
        auto& val = value["Forward"];
        if (val.is_boolean()) bForward = val.get<imgui_json::boolean>();
    }
    if (value.contains("Loop"))
    {
        auto& val = value["Loop"];
        if (val.is_boolean()) bLoop = val.get<imgui_json::boolean>();
    }
    if (value.contains("SelectLinked"))
    {
        auto& val = value["SelectLinked"];
        if (val.is_boolean()) bSelectLinked = val.get<imgui_json::boolean>();
    }
    
    Updata();
    //Seek();
    return 0;
}

void TimeLine::Save(imgui_json::value& value)
{
    // save media clip
    imgui_json::value media_clips;
    for (auto clip : m_Clips)
    {
        imgui_json::value media_clip;
        clip->Save(media_clip);
        media_clips.push_back(media_clip);
    }
    if (m_Clips.size() > 0) value["MediaClip"] = media_clips;

    // save clip group
    imgui_json::value clip_groups;
    for (auto group : m_Groups)
    {
        imgui_json::value media_group;
        group.Save(media_group);
        clip_groups.push_back(media_group);
    }
    if (m_Groups.size() > 0) value["MediaGroup"] = clip_groups;

    // save media overlap
    imgui_json::value overlaps;
    for (auto overlap : m_Overlaps)
    {
        imgui_json::value media_overlap;
        overlap->Save(media_overlap);
        overlaps.push_back(media_overlap);
    }
    if (m_Overlaps.size() > 0) value["MediaOverlap"] = overlaps;

    // save media track
    imgui_json::value media_tracks;
    for (auto track : m_Tracks)
    {
        imgui_json::value media_track;
        track->Save(media_track);
        media_tracks.push_back(media_track);
    }
    if (m_Tracks.size() > 0) value["MediaTrack"] = media_tracks;

    // save global timeline info
    value["Start"] = imgui_json::number(mStart);
    value["End"] = imgui_json::number(mEnd);
    value["VideoWidth"] = imgui_json::number(mWidth);
    value["VideoHeight"] = imgui_json::number(mHeight);
    value["FrameRateNum"] = imgui_json::number(mFrameRate.num);
    value["FrameRateDen"] = imgui_json::number(mFrameRate.den);
    value["AudioChannels"] = imgui_json::number(mAudioChannels);
    value["AudioSampleRate"] = imgui_json::number(mAudioSampleRate);
    value["AudioFormat"] = imgui_json::number(mAudioFormat);
    value["msPixelWidth"] = imgui_json::number(msPixelWidthTarget);
    value["FirstTime"] = imgui_json::number(firstTime);
    value["CurrentTime"] = imgui_json::number(currentTime);
    value["Forward"] = imgui_json::boolean(bForward);
    value["Loop"] = imgui_json::boolean(bLoop);
    value["SelectLinked"] = imgui_json::boolean(bSelectLinked);
}

} // namespace MediaTimeline/TimeLine

namespace MediaTimeline
{
/***********************************************************************************************************
 * Draw Main Timeline
 ***********************************************************************************************************/
bool DrawTimeLine(TimeLine *timeline, bool *expanded)
{
    /*************************************************************************************************************
     * [-]------------------------------------ header area ----------------------------------------------------- +
     *                    |  0    5    10 v   15    20 <rule bar> 30     35      40      45       50       55    c
     * ___________________|_______________|_____________________________________________________________________ a
     s | title     [+][-] |               |          Item bar                                                    n
     l |     legend       |---------------|--------------------------------------------------------------------- v
     o |                  |               |        custom area                                                   a 
     t |                  |               |                                                                      s                                            
     s |__________________|_______________|_____________________________________________________________________ |
     * | title     [+][-] |               |          Item bar                                                    h
     * |     legend       |---------------|--------------------------------------------------------------------- e
     * |                  |               |                                                                      i
     * |                  |               |        custom area                                                   g
     * |________________________________________________________________________________________________________ h
     * | title     [+][-] |               |          Item bar  <compact view>                                    t
     * |__________________|_______________|_____________________________________________________________________ +
     *                                                                                                           +
     *                     [     <==slider==>                                                                  ] +
     ++++++++++++++++++++++++++++++++++++++++ canvas width +++++++++++++++++++++++++++++++++++++++++++++++++++++++
     *************************************************************************************************************/
    bool ret = false;
    ImGuiIO &io = ImGui::GetIO();
    int cx = (int)(io.MousePos.x);
    int cy = (int)(io.MousePos.y);
    int scrollBarHeight = 16;
    int trackHeadHeight = 20;
    int HeadHeight = 20;
    int legendWidth = 200;
    int trackCount = timeline->GetTrackCount();
    int64_t duration = ImMax(timeline->GetEnd() - timeline->GetStart(), (int64_t)1);
    ImVector<TimelineCustomDraw> customDraws;
    static bool MovingScrollBar = false;
    static bool MovingCurrentTime = false;
    static int trackMovingEntry = -1;
    static int trackEntry = -1;
    static int64_t clipMovingEntry = -1;
    static int64_t clipEntry = -1;
    static int clipMovingPart = -1;
    int delTrackEntry = -1;
    int mouseEntry = -1;
    int legendEntry = -1;
    int64_t mouseClip = -1;
    int64_t mouseTime = -1;
    std::vector<int64_t> delClipEntry;
    std::vector<int64_t> groupClipEntry;
    std::vector<int64_t> unGroupClipEntry;
    bool removeEmptyTrack = false;
    static bool menuIsOpened = false;
    static bool bCutting = false;
    static bool bCropping = false;
    const bool is_alt_key_only = (io.KeyMods == ImGuiKeyModFlags_Alt);
    bCutting = ImGui::IsKeyDown(ImGuiKey_LeftAlt) && is_alt_key_only;
    
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetCursorScreenPos();
    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();                    // ImDrawList API uses screen coordinates!
    ImVec2 canvas_size = ImGui::GetContentRegionAvail() - ImVec2(8, 0); // Resize canvas to what's available

    // zoom in/out
    static bool panningView = false;
    static ImVec2 panningViewSource;
    static int64_t panningViewTime;
    timeline->visibleTime = (int64_t)floorf((canvas_size.x - legendWidth) / timeline->msPixelWidthTarget);
    const float barWidthRatio = ImMin(timeline->visibleTime / (float)duration, 1.f);
    const float barWidthInPixels = barWidthRatio * (canvas_size.x - legendWidth);
    ImRect regionRect(canvas_pos + ImVec2(0, HeadHeight), canvas_pos + canvas_size);
    ImRect scrollBarRect;
    ImRect scrollHandleBarRect;

    float minPixelWidthTarget = ImMin(timeline->msPixelWidthTarget, (float)(canvas_size.x - legendWidth) / (float)duration);
    float frame_duration = (timeline->mFrameRate.den > 0 && timeline->mFrameRate.num > 0) ? timeline->mFrameRate.den * 1000.0 / timeline->mFrameRate.num : 40;
    float maxPixelWidthTarget = frame_duration > 0.0 ? 60.f / frame_duration : 20.f;
    timeline->msPixelWidthTarget = ImClamp(timeline->msPixelWidthTarget, minPixelWidthTarget, maxPixelWidthTarget);

    if (timeline->visibleTime >= duration)
        timeline->firstTime = timeline->GetStart();
    timeline->lastTime = timeline->firstTime + timeline->visibleTime;
    
    int controlHeight = trackCount * trackHeadHeight;
    for (int i = 0; i < trackCount; i++)
        controlHeight += int(timeline->GetCustomHeight(i));

    // draw backbround
    draw_list->AddRectFilled(window_pos, window_pos + window_size, COL_DARK_TWO);

    ImGui::BeginGroup();
    bool isFocused = ImGui::IsWindowFocused();

    ImGui::SetCursorScreenPos(canvas_pos);
    if ((expanded && !*expanded) || !trackCount)
    {
        // minimum view
        ImGui::InvisibleButton("canvas_minimum", ImVec2(canvas_size.x - canvas_pos.x, (float)HeadHeight));
        draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + HeadHeight), COL_DARK_ONE, 0);
        auto info_str = MillisecToString(duration, 3);
        info_str += " / ";
        info_str += std::to_string(trackCount) + " entries";
        draw_list->AddText(ImVec2(canvas_pos.x + 40, canvas_pos.y + 2), IM_COL32_WHITE, info_str.c_str());
    }
    else
    {
        // normal view
        ImVec2 headerSize(canvas_size.x - 4.f, (float)HeadHeight);
        ImVec2 scrollBarSize(canvas_size.x, scrollBarHeight);
        ImGui::InvisibleButton("topBar", headerSize);
        draw_list->AddRectFilled(canvas_pos, canvas_pos + headerSize, COL_DARK_ONE, 0);
        if (!trackCount) 
        {
            ImGui::EndGroup();
            return false;
        }
        ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
        ImVec2 childFramePos = ImGui::GetCursorScreenPos();
        ImVec2 childFrameSize(canvas_size.x, canvas_size.y - 8.0f - headerSize.y - scrollBarSize.y);
        ImGui::BeginChildFrame(ImGui::GetID("timeline_Tracks"), childFrameSize, ImGuiWindowFlags_NoScrollbar);

        ImGui::InvisibleButton("contentBar", ImVec2(canvas_size.x - 8.f, float(controlHeight)));
        const ImVec2 contentMin = ImGui::GetItemRectMin();
        const ImVec2 contentMax = ImGui::GetItemRectMax();
        const ImRect contentRect(contentMin, contentMax);
        const ImRect legendRect(contentMin, ImVec2(contentMin.x + legendWidth, contentMax.y));
        const ImRect legendAreaRect(contentMin, ImVec2(contentMin.x + legendWidth, contentMin.y + canvas_size.y - (HeadHeight + 8)));
        const ImRect trackRect(ImVec2(contentMin.x + legendWidth, contentMin.y), contentMax);
        const ImRect trackAreaRect(ImVec2(contentMin.x + legendWidth, contentMin.y), ImVec2(contentMax.x, contentMin.y + canvas_size.y - (HeadHeight + scrollBarHeight + 8)));
        
        const float contentHeight = contentMax.y - contentMin.y;
        // full canvas background
        draw_list->AddRectFilled(canvas_pos + ImVec2(4, HeadHeight + 4), canvas_pos + ImVec2(4, HeadHeight + 4) + canvas_size - ImVec2(8, HeadHeight + scrollBarHeight + 8), COL_CANVAS_BG, 0);
        // full legend background
        draw_list->AddRectFilled(legendRect.Min, legendRect.Max, COL_LEGEND_BG, 0);

        // for debug
        //draw_list->AddRect(trackRect.Min, trackRect.Max, IM_COL32(255, 0, 0, 255), 0, 0, 2);
        //draw_list->AddRect(trackAreaRect.Min, trackAreaRect.Max, IM_COL32(0, 0, 255, 255), 0, 0, 2);
        //draw_list->AddRect(legendRect.Min, legendRect.Max, IM_COL32(0, 255, 0, 255), 0, 0, 2);
        //draw_list->AddRect(legendAreaRect.Min, legendAreaRect.Max, IM_COL32(255, 255, 0, 255), 0, 0, 2);
        // for debug end

        // current time top
        ImRect topRect(ImVec2(canvas_pos.x + legendWidth, canvas_pos.y), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + HeadHeight));
        if (!MovingCurrentTime && !MovingScrollBar && clipMovingEntry == -1 && timeline->currentTime >= 0 && topRect.Contains(io.MousePos) && io.MouseDown[0] && isFocused)
        {
            MovingCurrentTime = true;
            timeline->bSeeking = true;
        }
        if (MovingCurrentTime && duration)
        {
            auto old_time = timeline->currentTime;
            timeline->currentTime = (int64_t)((io.MousePos.x - topRect.Min.x) / timeline->msPixelWidthTarget) + timeline->firstTime;
            timeline->AlignTime(timeline->currentTime);
            if (timeline->currentTime < timeline->GetStart())
                timeline->currentTime = timeline->GetStart();
            if (timeline->currentTime >= timeline->GetEnd())
                timeline->currentTime = timeline->GetEnd();
            //if (old_time != timeline->currentTime)
            //    timeline->Seek(); // call seek event
        }
        if (timeline->bSeeking && !io.MouseDown[0])
        {
            MovingCurrentTime = false;
            timeline->bSeeking = false;
        }

        // calculate mouse pos to time
        mouseTime = (int64_t)((cx - topRect.Min.x) / timeline->msPixelWidthTarget) + timeline->firstTime;
        timeline->AlignTime(mouseTime);
        menuIsOpened = ImGui::IsPopupOpen("##timeline-context-menu");

        //header
        //header time and lines
        int64_t modTimeCount = 10;
        int timeStep = 1;
        while ((modTimeCount * timeline->msPixelWidthTarget) < 75)
        {
            modTimeCount *= 10;
            timeStep *= 10;
        };
        int halfModTime = modTimeCount / 2;
        auto drawLine = [&](int64_t i, int regionHeight)
        {
            bool baseIndex = ((i % modTimeCount) == 0) || (i == timeline->GetEnd() || i == timeline->GetStart());
            bool halfIndex = (i % halfModTime) == 0;
            int px = (int)canvas_pos.x + int(i * timeline->msPixelWidthTarget) + legendWidth - int(timeline->firstTime * timeline->msPixelWidthTarget);
            int tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
            int tiretEnd = baseIndex ? regionHeight : HeadHeight;
            if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
            {
                draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart), ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), halfIndex ? COL_MARK : COL_MARK_HALF, halfIndex ? 2 : 1);
            }
            if (baseIndex && px > (canvas_pos.x + legendWidth))
            {
                auto time_str = MillisecToString(i, 2);
                ImGui::SetWindowFontScale(0.8);
                draw_list->AddText(ImVec2((float)px + 3.f, canvas_pos.y), COL_RULE_TEXT, time_str.c_str());
                ImGui::SetWindowFontScale(1.0);
            }
        };
        auto drawLineContent = [&](int64_t i, int /*regionHeight*/)
        {
            int px = (int)canvas_pos.x + int(i * timeline->msPixelWidthTarget) + legendWidth - int(timeline->firstTime * timeline->msPixelWidthTarget);
            int tiretStart = int(contentMin.y);
            int tiretEnd = int(contentMax.y);
            if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
            {
                draw_list->AddLine(ImVec2(float(px), float(tiretStart)), ImVec2(float(px), float(tiretEnd)), COL_SLOT_V_LINE, 1);
            }
        };
        for (auto i = timeline->GetStart(); i <= timeline->GetEnd(); i += timeStep)
        {
            drawLine(i, HeadHeight);
        }
        drawLine(timeline->GetStart(), HeadHeight);
        drawLine(timeline->GetEnd(), HeadHeight);

        // cursor Arrow
        if (timeline->currentTime >= timeline->firstTime && timeline->currentTime <= timeline->GetEnd())
        {
            const float arrowWidth = draw_list->_Data->FontSize;
            float arrowOffset = contentMin.x + legendWidth + (timeline->currentTime - timeline->firstTime) * timeline->msPixelWidthTarget + timeline->msPixelWidthTarget / 2 - arrowWidth * 0.5f - 3;
            ImGui::RenderArrow(draw_list, ImVec2(arrowOffset, canvas_pos.y), COL_CURSOR_ARROW, ImGuiDir_Down);
            ImGui::SetWindowFontScale(0.8);
            auto time_str = MillisecToString(timeline->currentTime, 2);
            ImVec2 str_size = ImGui::CalcTextSize(time_str.c_str(), nullptr, true);
            float strOffset = contentMin.x + legendWidth + (timeline->currentTime - timeline->firstTime) * timeline->msPixelWidthTarget + timeline->msPixelWidthTarget / 2 - str_size.x * 0.5f - 3;
            ImVec2 str_pos = ImVec2(strOffset, canvas_pos.y + 10);
            draw_list->AddRectFilled(str_pos + ImVec2(-3, 0), str_pos + str_size + ImVec2(3, 3), COL_CURSOR_TEXT_BG, 2.0, ImDrawFlags_RoundCornersAll);
            draw_list->AddText(str_pos, COL_CURSOR_TEXT, time_str.c_str());
            ImGui::SetWindowFontScale(1.0);
        }

        // crop content
        draw_list->PushClipRect(childFramePos, childFramePos + childFrameSize);

        // track background
        size_t customHeight = 0;
        for (int i = 0; i < trackCount; i++)
        {
            unsigned int col = (i & 1) ? COL_SLOT_ODD : COL_SLOT_EVEN;
            size_t localCustomHeight = timeline->GetCustomHeight(i);
            ImVec2 pos = ImVec2(contentMin.x + legendWidth, contentMin.y + trackHeadHeight * i + 1 + customHeight);
            ImVec2 sz = ImVec2(canvas_size.x + canvas_pos.x - 4.f, pos.y + trackHeadHeight - 1 + localCustomHeight);
            if (cy >= pos.y && cy < pos.y + (trackHeadHeight + localCustomHeight) && clipMovingEntry == -1 && cx > contentMin.x && cx < contentMin.x + canvas_size.x)
            {
                col += IM_COL32(8, 16, 32, 128);
                pos.x -= legendWidth;
            }
            draw_list->AddRectFilled(pos, sz, col, 0);
            customHeight += localCustomHeight;
        }

        // draw track legend area title bar
        customHeight = 0;
        for (int i = 0; i < trackCount; i++)
        {
            auto itemCustomHeight = timeline->GetCustomHeight(i);
            ImVec2 button_size = ImVec2(14, 14);
            ImVec2 tpos(contentMin.x, contentMin.y + i * trackHeadHeight + customHeight);
            bool ret = TimelineButton(draw_list, ICON_TRASH, ImVec2(contentMin.x + legendWidth - button_size.x - 4, tpos.y + 2), button_size, "delete");
            if (ret && io.MouseClicked[0])
                delTrackEntry = i;
            customHeight += itemCustomHeight;
        }

        // cropping rect so track bars are not visible in the legend on the left when scrolled
        draw_list->PushClipRect(childFramePos + ImVec2(float(legendWidth), 0.f), childFramePos + childFrameSize);
        // vertical time lines in content area
        for (auto i = timeline->GetStart(); i <= timeline->GetEnd(); i += timeStep)
        {
            drawLineContent(i, int(contentHeight));
        }
        drawLineContent(timeline->GetStart(), int(contentHeight));
        drawLineContent(timeline->GetEnd(), int(contentHeight));

        // track
        customHeight = 0;
        for (int i = 0; i < trackCount; i++)
        {
            size_t localCustomHeight = timeline->GetCustomHeight(i);
            unsigned int color = COL_SLOT_DEFAULT;
            ImVec2 pos = ImVec2(contentMin.x + legendWidth - timeline->firstTime * timeline->msPixelWidthTarget, contentMin.y + trackHeadHeight * i + 1 + customHeight);
            ImVec2 slotP1(pos.x + timeline->firstTime * timeline->msPixelWidthTarget, pos.y + 2);
            ImVec2 slotP2(pos.x + timeline->lastTime * timeline->msPixelWidthTarget + timeline->msPixelWidthTarget - 4.f, pos.y + trackHeadHeight - 2);
            ImVec2 slotP3(pos.x + timeline->lastTime * timeline->msPixelWidthTarget + timeline->msPixelWidthTarget - 4.f, pos.y + trackHeadHeight - 2 + localCustomHeight);
            unsigned int slotColor = color | IM_COL32_BLACK;
            unsigned int slotColorHalf = HALF_COLOR(color);
            if (slotP1.x <= (canvas_size.x + contentMin.x) && slotP2.x >= (contentMin.x + legendWidth))
            {
                draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 0);
                draw_list->AddRectFilled(slotP1, slotP2, slotColor, 0);
            }
            if (ImRect(slotP1, slotP2).Contains(io.MousePos))
            {
                if (io.MouseDoubleClicked[0])
                    timeline->DoubleClick(i, mouseTime);
            }
            if (ImRect(slotP1, slotP3).Contains(io.MousePos))
            {
                mouseEntry = i;
            }
            ImVec2 slotLegendP1(contentMin.x, contentMin.y + trackHeadHeight * i + customHeight);
            ImVec2 slotLegendP2(slotLegendP1.x + legendWidth, slotLegendP1.y + trackHeadHeight + localCustomHeight);
            if (ImRect(slotLegendP1, slotLegendP2).Contains(io.MousePos))
            {
                legendEntry = i;
            }

            // Ensure grabable handles for track
            if (legendEntry != -1 && legendEntry < timeline->m_Tracks.size())
            {
                if (ImGui::IsMouseClicked(0) && !MovingScrollBar && !MovingCurrentTime && !menuIsOpened)
                {
                    trackMovingEntry = legendEntry;
                }
            }

            // Ensure grabable handles and find selected clip
            if (mouseTime != -1 && mouseEntry >= 0 && mouseEntry < timeline->m_Tracks.size())
            {
                MediaTrack * track = timeline->m_Tracks[mouseEntry];
                if (track && !track->mLocked)
                {
                    int count = 0;
                    auto clip = track->FindClips(mouseTime, count);
                    if (clip && clipMovingEntry == -1)
                    {
                        // check clip moving part
                        ImVec2 clipP1(pos.x + clip->mStart * timeline->msPixelWidthTarget, pos.y + 2);
                        ImVec2 clipP2(pos.x + clip->mEnd * timeline->msPixelWidthTarget + timeline->msPixelWidthTarget - 4.f, pos.y + trackHeadHeight - 2);
                        ImVec2 clipP3(pos.x + clip->mEnd * timeline->msPixelWidthTarget + timeline->msPixelWidthTarget - 4.f, pos.y + trackHeadHeight - 2 + localCustomHeight);
                        const float max_handle_width = clipP2.x - clipP1.x / 3.0f;
                        const float min_handle_width = ImMin(10.0f, max_handle_width);
                        const float handle_width = ImClamp(timeline->msPixelWidthTarget / 2.0f, min_handle_width, max_handle_width);
                        ImRect rects[3] = {ImRect(clipP1, ImVec2(clipP1.x + handle_width, clipP2.y)), ImRect(ImVec2(clipP2.x - handle_width, clipP1.y), clipP2), ImRect(clipP1, clipP3)};

                        for (int j = 1; j >= 0; j--)
                        {
                            ImRect &rc = rects[j];
                            if (!rc.Contains(io.MousePos))
                                continue;
                            if (j == 0)
                                RenderMouseCursor(ICON_CROPPING_LEFT, ImVec2(4, 0));
                            else
                                RenderMouseCursor(ICON_CROPPING_RIGHT, ImVec2(12, 0));
                            draw_list->AddRectFilled(rc.Min, rc.Max, IM_COL32(255,0,0,255), 0);
                        }
                        for (int j = 0; j < 3; j++)
                        {
                            ImRect &rc = rects[j];
                            if (!rc.Contains(io.MousePos))
                                continue;
                            if (!ImRect(childFramePos, childFramePos + childFrameSize).Contains(io.MousePos))
                                continue;
                            if (j == 2 && bCutting && count <= 1)
                            {
                                // draw dotted line at mouse pos
                                ImVec2 P1(cx, canvas_pos.y + (float)HeadHeight + 8.f);
                                ImVec2 P2(cx, canvas_pos.y + (float)HeadHeight + float(controlHeight) + 8.f);
                                draw_list->AddLine(P1, P2, IM_COL32(0, 0, 255, 255), 2);
                                RenderMouseCursor(ICON_CUTTING, ImVec2(7, 0), 1.0, -90);
                            }
                            if (ImGui::IsMouseClicked(0) && !MovingScrollBar && !MovingCurrentTime && !menuIsOpened)
                            {
                                if (j == 2 && bCutting && count <= 1)
                                {
                                    clip->Cutting(mouseTime);
                                }
                                else
                                {
                                    clipMovingEntry = clip->mID;
                                    clipMovingPart = j + 1;
                                    if (j <= 1)
                                        bCropping = true;
                                }
                                break;
                            }
                        }
                    }
                }
                if (track)
                {
                    for (auto clip : track->m_Clips)
                    {
                        if (clip->mStart <= mouseTime && clip->mEnd >= mouseTime)
                        {
                            mouseClip = clip->mID;
                            break;
                        }
                    }
                }
            }
            customHeight += localCustomHeight;
        }

        // check mouseEntry is below track area
        if (mouseEntry < 0 && trackAreaRect.Contains(io.MousePos) && cy >= trackRect.Max.y)
        {
            mouseEntry = -2;
        }

        // track moving
        if (trackMovingEntry != -1 && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            ImGui::CaptureMouseFromApp();
            if (cy > legendAreaRect.Min.y && cy < legendAreaRect.Max.y)
            {
                trackEntry = legendEntry;
                if (trackEntry == -1 && cy >= legendRect.Max.y)
                {
                    trackEntry = -2; // end of tracks
                }
            }
            //ImGui::BeginTooltip();
            //ImGui::Text("Draging track:%s", std::to_string(trackMovingEntry).c_str());
            //ImGui::Text("currrent track:%s", std::to_string(trackEntry).c_str());
            //ImGui::EndTooltip();
        }

        // clip cropping or moving
        if (clipMovingEntry != -1 && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            ImGui::CaptureMouseFromApp();
            int64_t diffTime = int64_t(io.MouseDelta.x / timeline->msPixelWidthTarget);
            if (std::abs(diffTime) > 0)
            {
                Clip * clip = timeline->FindClipByID(clipMovingEntry);
                if (clip && clip->bSelected)
                {
                    if (clipMovingPart == 3)
                    {
                        // whole slot moving
                        int dst_entry = clip->Moving(diffTime, mouseEntry);
                        if (dst_entry >= 0)
                            mouseEntry = dst_entry;
                    }
                    else if (clipMovingPart & 1)
                    {
                        // clip left moving
                        clip->Cropping(diffTime, 0);
                    }
                    else if (clipMovingPart & 2)
                    {
                        // clip right moving
                        clip->Cropping(diffTime, 1);
                    }
                }
            }
        }
        if (!io.MouseDown[0])
        {
            clipMovingEntry = -1;
            clipMovingPart = -1;
            trackMovingEntry = -1;
            trackEntry = -1;
            bCropping = false;
            ImGui::CaptureMouseFromApp(false);
        }

        if (bCropping)
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }

        if (!menuIsOpened && !bCropping && !bCutting && io.MouseClicked[0] && !io.MouseDoubleClicked[0])
            timeline->Click(mouseEntry, mouseTime);

        if (io.MouseClicked[1] && !io.MouseDoubleClicked[1])
        {
            if (mouseClip != -1)
                clipEntry = mouseClip;
            ImGui::OpenPopup("##timeline-context-menu");
            menuIsOpened = true;
        }

        draw_list->PopClipRect();
        draw_list->PopClipRect();

        // handle menu
        if (!menuIsOpened)
        {
            clipEntry = -1;
        }

        if (ImGui::BeginPopup("##timeline-context-menu"))
        {
            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(255,255,255,255));
            auto selected_clip_count = timeline->GetSelectedClipCount();
            if (ImGui::MenuItem("Delete Empty Track", nullptr, nullptr))
            {
                removeEmptyTrack = true;
                menuIsOpened = false;
            }
            if (clipEntry != -1)
            {
                auto clip = timeline->FindClipByID(clipEntry);
                if (ImGui::MenuItem("Delete Clip", nullptr, nullptr))
                {
                    delClipEntry.push_back(clipEntry);
                    menuIsOpened = false;
                }
                if (clip->mGroupID != -1 && ImGui::MenuItem("Ungroup Clip", nullptr, nullptr))
                {
                    unGroupClipEntry.push_back(clipEntry);
                    menuIsOpened = false;
                }
            }
            if (selected_clip_count > 0)
            {
                ImGui::Separator();
                if (ImGui::MenuItem("Delete Selected", nullptr, nullptr))
                {
                    for (auto clip : timeline->m_Clips)
                    {
                        if (clip->bSelected)
                            delClipEntry.push_back(clip->mID);
                    }
                    menuIsOpened = false;
                }
                if (selected_clip_count > 1 && ImGui::MenuItem("Group Selected", nullptr, nullptr))
                {
                    for (auto clip : timeline->m_Clips)
                    {
                        if (clip->bSelected)
                            groupClipEntry.push_back(clip->mID);
                    }
                    menuIsOpened = false;
                }
                if (ImGui::MenuItem("Ungroup Selected", nullptr, nullptr))
                {
                    for (auto clip : timeline->m_Clips)
                    {
                        if (clip->bSelected)
                            unGroupClipEntry.push_back(clip->mID);
                    }
                    menuIsOpened = false;
                }
            }
            ImGui::PopStyleColor();
            ImGui::EndPopup();
        }
        ImGui::EndChildFrame();

        // Scroll bar control buttons
        auto scroll_pos = ImGui::GetCursorScreenPos();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::SetWindowFontScale(0.7);
        int button_offset = 16;
        ImGui::SetCursorScreenPos(scroll_pos + ImVec2(legendWidth - button_offset - 4, 0));
        if (ImGui::Button(ICON_FAST_TO_END "##slider_to_end", ImVec2(16, 16)))
        {
            timeline->firstTime = timeline->GetEnd() - timeline->visibleTime;
            timeline->AlignTime(timeline->firstTime);
        }
        ImGui::ShowTooltipOnHover("Slider to End");

        button_offset += 16;
        ImGui::SetCursorScreenPos(scroll_pos + ImVec2(legendWidth - button_offset - 4, 0));
        if (ImGui::Button(ICON_SLIDER_MAXIMUM "##slider_maximum", ImVec2(16, 16)))
        {
            timeline->msPixelWidthTarget = maxPixelWidthTarget;
        }
        ImGui::ShowTooltipOnHover("Maximum Slider");

        button_offset += 16;
        ImGui::SetCursorScreenPos(scroll_pos + ImVec2(legendWidth - button_offset - 4, 0));
        if (ImGui::Button(ICON_ZOOM_IN "##slider_zoom_in", ImVec2(16, 16)))
        {
            timeline->msPixelWidthTarget *= 2.0f;
            if (timeline->msPixelWidthTarget > maxPixelWidthTarget)
                timeline->msPixelWidthTarget = maxPixelWidthTarget;
        }
        ImGui::ShowTooltipOnHover("Slider Zoom In");

        button_offset += 16;
        ImGui::SetCursorScreenPos(scroll_pos + ImVec2(legendWidth - button_offset - 4, 0));
        if (ImGui::Button(ICON_ZOOM_OUT "##slider_zoom_out", ImVec2(16, 16)))
        {
            timeline->msPixelWidthTarget *= 0.5f;
            if (timeline->msPixelWidthTarget < minPixelWidthTarget)
                timeline->msPixelWidthTarget = minPixelWidthTarget;
        }
        ImGui::ShowTooltipOnHover("Slider Zoom Out");

        button_offset += 16;
        ImGui::SetCursorScreenPos(scroll_pos + ImVec2(legendWidth - button_offset - 4, 0));
        if (ImGui::Button(ICON_SLIDER_MINIMUM "##slider_minimum", ImVec2(16, 16)))
        {
            timeline->msPixelWidthTarget = minPixelWidthTarget;
            timeline->firstTime = timeline->GetStart();
        }
        ImGui::ShowTooltipOnHover("Minimum Slider");

        button_offset += 16;
        ImGui::SetCursorScreenPos(scroll_pos + ImVec2(legendWidth - button_offset - 4, 0));
        if (ImGui::Button(ICON_FAST_TO_START "##slider_to_start", ImVec2(16, 16)))
        {
            timeline->firstTime = timeline->GetStart();
            timeline->AlignTime(timeline->firstTime);
        }
        ImGui::ShowTooltipOnHover("Slider to Start");
        ImGui::SetWindowFontScale(1.0);
        ImGui::PopStyleColor();

        // Scroll bar
        ImGui::SetCursorScreenPos(scroll_pos);
        ImGui::InvisibleButton("scrollBar", scrollBarSize);
        ImVec2 scrollBarMin = ImGui::GetItemRectMin();
        ImVec2 scrollBarMax = ImGui::GetItemRectMax();
        float startOffset = ((float)(timeline->firstTime - timeline->GetStart()) / (float)duration) * (canvas_size.x - legendWidth);
        ImVec2 scrollBarA(scrollBarMin.x + legendWidth, scrollBarMin.y - 2);
        ImVec2 scrollBarB(scrollBarMin.x + canvas_size.x, scrollBarMax.y - 1);
        scrollBarRect = ImRect(scrollBarA, scrollBarB);
        bool inScrollBar = scrollBarRect.Contains(io.MousePos);
        draw_list->AddRectFilled(scrollBarA, scrollBarB, COL_SLIDER_BG, 8);
        ImVec2 scrollBarC(scrollBarMin.x + legendWidth + startOffset, scrollBarMin.y);
        ImVec2 scrollBarD(scrollBarMin.x + legendWidth + barWidthInPixels + startOffset, scrollBarMax.y - 2);
        scrollHandleBarRect = ImRect(scrollBarC, scrollBarD);
        bool inScrollHandle = scrollHandleBarRect.Contains(io.MousePos);
        draw_list->AddRectFilled(scrollBarC, scrollBarD, (inScrollBar || MovingScrollBar) ? COL_SLIDER_IN : COL_SLIDER_MOVING, 6);
        if (MovingScrollBar)
        {
            if (!io.MouseDown[0])
            {
                MovingScrollBar = false;
            }
            else
            {
                float msPerPixelInBar = barWidthInPixels / (float)timeline->visibleTime;
                timeline->firstTime = int((io.MousePos.x - panningViewSource.x) / msPerPixelInBar) - panningViewTime;
                timeline->AlignTime(timeline->firstTime);
                timeline->firstTime = ImClamp(timeline->firstTime, timeline->GetStart(), ImMax(timeline->GetEnd() - timeline->visibleTime, timeline->GetStart()));
            }
        }
        else if (inScrollHandle && ImGui::IsMouseClicked(0) && !MovingCurrentTime && clipMovingEntry == -1)
        {
            MovingScrollBar = true;
            panningViewSource = io.MousePos;
            panningViewTime = - timeline->firstTime;
        }
        else if (inScrollBar && ImGui::IsMouseReleased(0))
        {
            float msPerPixelInBar = barWidthInPixels / (float)timeline->visibleTime;
            timeline->firstTime = int((io.MousePos.x - legendWidth - scrollHandleBarRect.GetWidth() / 2)/ msPerPixelInBar);
            timeline->AlignTime(timeline->firstTime);
            timeline->firstTime = ImClamp(timeline->firstTime, timeline->GetStart(), ImMax(timeline->GetEnd() - timeline->visibleTime, timeline->GetStart()));
        }

        // handle mouse wheel event
        if (regionRect.Contains(io.MousePos))
        {
            bool overTrackView = false;
            bool overScrollBar = false;
            bool overCustomDraw = false;
            if (trackRect.Contains(io.MousePos))
            {
                overCustomDraw = true;
            }
            if (trackAreaRect.Contains(io.MousePos))
            {
                overTrackView = true;
            } 
            if (scrollBarRect.Contains(io.MousePos))
            {
                overScrollBar = true;
            }
            if (overScrollBar)
            {
                // up-down wheel over scrollbar, scale canvas view
                int64_t overCursor = timeline->firstTime + (int64_t)(timeline->visibleTime * ((io.MousePos.x - (float)legendWidth - canvas_pos.x) / (canvas_size.x - legendWidth)));
                if (io.MouseWheel < -FLT_EPSILON && timeline->visibleTime <= timeline->GetEnd())
                {
                    timeline->msPixelWidthTarget *= 0.9f;
                }
                else if (io.MouseWheel > FLT_EPSILON)
                {
                    timeline->msPixelWidthTarget *= 1.1f;
                }
            }
            if (overTrackView || overScrollBar)
            {
                // left-right wheel over blank area, moving canvas view
                if (io.MouseWheelH < -FLT_EPSILON)
                {
                    timeline->firstTime -= timeline->visibleTime / 4;
                    timeline->firstTime = ImClamp(timeline->firstTime, timeline->GetStart(), ImMax(timeline->GetEnd() - timeline->visibleTime, timeline->GetStart()));
                }
                else if (io.MouseWheelH > FLT_EPSILON)
                {
                    timeline->firstTime += timeline->visibleTime / 4;
                    timeline->firstTime = ImClamp(timeline->firstTime, timeline->GetStart(), ImMax(timeline->GetEnd() - timeline->visibleTime, timeline->GetStart()));
                }
            }
        }

        // calculate custom draw rect
        customHeight = 0;
        for (int i = 0; i < trackCount; i++)
        {
            int64_t start = 0, end = 0, length = 0;
            size_t localCustomHeight = timeline->GetCustomHeight(i);

            ImVec2 rp(canvas_pos.x, contentMin.y + trackHeadHeight * i + 1 + customHeight);
            ImRect customRect(rp + ImVec2(legendWidth - (timeline->firstTime - start - 0.5f) * timeline->msPixelWidthTarget, float(trackHeadHeight)),
                              rp + ImVec2(legendWidth + (end - timeline->firstTime - 0.5f + 2.f) * timeline->msPixelWidthTarget, float(localCustomHeight + trackHeadHeight)));
            ImRect titleRect(rp + ImVec2(legendWidth - (timeline->firstTime - start - 0.5f) * timeline->msPixelWidthTarget, 0),
                              rp + ImVec2(legendWidth + (end - timeline->firstTime - 0.5f + 2.f) * timeline->msPixelWidthTarget, float(trackHeadHeight)));
            ImRect clippingTitleRect(rp + ImVec2(float(legendWidth), 0), rp + ImVec2(canvas_size.x - 4.0f, float(trackHeadHeight)));
            ImRect clippingRect(rp + ImVec2(float(legendWidth), float(trackHeadHeight)), rp + ImVec2(canvas_size.x - 4.0f, float(localCustomHeight + trackHeadHeight)));
            ImRect legendRect(rp, rp + ImVec2(float(legendWidth), float(localCustomHeight + trackHeadHeight)));
            ImRect legendClippingRect(rp + ImVec2(0.f, float(trackHeadHeight)), rp + ImVec2(float(legendWidth), float(localCustomHeight + trackHeadHeight)));
            customDraws.push_back({i, customRect, titleRect, clippingTitleRect, legendRect, clippingRect, legendClippingRect});

            customHeight += localCustomHeight;
        }

        // draw custom
        draw_list->PushClipRect(childFramePos, childFramePos + childFrameSize);
        for (auto &customDraw : customDraws)
            timeline->CustomDraw(customDraw.index, draw_list, customDraw.customRect, customDraw.titleRect, customDraw.clippingTitleRect, customDraw.legendRect, customDraw.clippingRect, customDraw.legendClippingRect, timeline->firstTime, timeline->visibleTime, timeline->msPixelWidthTarget, true, !menuIsOpened && !bCutting);
        draw_list->PopClipRect();

        // cursor line
        ImRect custom_view_rect(childFramePos + ImVec2(float(legendWidth), 0.f), childFramePos + childFrameSize);
        draw_list->PushClipRect(custom_view_rect.Min, custom_view_rect.Max);
        if (trackCount > 0 && timeline->currentTime >= timeline->firstTime && timeline->currentTime <= timeline->GetEnd())
        {
            ImVec2 contentMin(canvas_pos.x + 4.f, canvas_pos.y + (float)HeadHeight + 8.f);
            ImVec2 contentMax(canvas_pos.x + canvas_size.x - 4.f, canvas_pos.y + (float)HeadHeight + float(controlHeight) + 8.f);
            static const float cursorWidth = 3.f;
            float cursorOffset = contentMin.x + legendWidth + (timeline->currentTime - timeline->firstTime) * timeline->msPixelWidthTarget + timeline->msPixelWidthTarget / 2 - cursorWidth * 0.5f - 2;
            draw_list->AddLine(ImVec2(cursorOffset, contentMin.y), ImVec2(cursorOffset, contentMax.y), IM_COL32(0, 255, 0, 128), cursorWidth);
        }
        draw_list->PopClipRect();

        ImGui::PopStyleColor();
    }
    ImGui::EndGroup();

    // handle drag drop
    ImGui::SetCursorScreenPos(canvas_pos + ImVec2(4, trackHeadHeight + 4));
    ImGui::InvisibleButton("canvas", canvas_size - ImVec2(8, trackHeadHeight + scrollBarHeight + 8));
    if (ImGui::BeginDragDropTarget())
    {
        // find current mouse pos track
        MediaTrack * track = nullptr;
        if (mouseEntry >= 0 && mouseEntry < timeline->m_Tracks.size())
        {
            track = timeline->m_Tracks[mouseEntry];
        }
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Media_drag_drop"))
        {
            MediaItem * item = (MediaItem*)payload->Data;
            if (item)
            {
                if (item->mMediaType == MEDIA_PICTURE)
                {
                    ImageClip * new_image_clip = new ImageClip(item->mStart, item->mEnd, item->mID, item->mName, item->mMediaOverview, timeline);
                    timeline->m_Clips.push_back(new_image_clip);
                    if (track && track->mType == MEDIA_PICTURE)
                    {
                        // update clip info and push into track
                        track->InsertClip(new_image_clip, mouseTime);
                        timeline->Updata();
                    }
                    else
                    {
                        MediaTrack * new_track = new MediaTrack("", MEDIA_PICTURE, timeline);
                        new_track->mExpanded = true;
                        new_track->InsertClip(new_image_clip, mouseTime);
                        timeline->m_Tracks.push_back(new_track);
                        timeline->Updata();
                    }
                }
                else if (item->mMediaType == MEDIA_AUDIO)
                {
                    AudioClip * new_audio_clip = new AudioClip(item->mStart, item->mEnd, item->mID, item->mName, item->mMediaOverview, timeline);
                    timeline->m_Clips.push_back(new_audio_clip);
                    if (track && track->mType == MEDIA_AUDIO)
                    {
                        // update clip info and push into track
                        track->InsertClip(new_audio_clip, mouseTime);
                        timeline->Updata();
                    }
                    else
                    {
                        MediaTrack * new_track = new MediaTrack("", MEDIA_AUDIO, timeline);
                        new_track->mExpanded = true;
                        new_track->InsertClip(new_audio_clip, mouseTime);
                        timeline->m_Tracks.push_back(new_track);
                        timeline->Updata();
                    }
                } 
                else if (item->mMediaType == MEDIA_TEXT)
                {
                    TextClip * new_text_clip = new TextClip(item->mStart, item->mEnd, item->mID, item->mName, item->mMediaOverview, timeline);
                    timeline->m_Clips.push_back(new_text_clip);
                    // TODO::Dicky add text support
                }
                else
                {
                    bool create_new_track = false;
                    MediaTrack * video_track = nullptr;
                    VideoClip * new_video_clip = nullptr;
                    AudioClip * new_audio_clip = nullptr;
                    const MediaInfo::VideoStream* video_stream = item->mMediaOverview->GetVideoStream();
                    const MediaInfo::AudioStream* audio_stream = item->mMediaOverview->GetAudioStream();
                    if (video_stream)
                    {
                        new_video_clip = new VideoClip(item->mStart, item->mEnd, item->mID, item->mName, item->mMediaOverview, timeline);
                        timeline->m_Clips.push_back(new_video_clip);
                        if (track && track->mType == MEDIA_VIDEO)
                        {
                            // update clip info and push into track
                            track->InsertClip(new_video_clip, mouseTime);
                            timeline->Updata();
                        }
                        else
                        {
                            video_track = new MediaTrack("", MEDIA_VIDEO, timeline);
                            video_track->mExpanded = true;
                            video_track->InsertClip(new_video_clip, mouseTime);
                            timeline->m_Tracks.push_back(video_track);
                            timeline->Updata();
                            create_new_track = true;
                        }
                    }
                    if (audio_stream)
                    {
                        new_audio_clip = new AudioClip(item->mStart, item->mEnd, item->mID, item->mName, item->mMediaOverview, timeline);
                        timeline->m_Clips.push_back(new_audio_clip);
                        if (!create_new_track)
                        {
                            if (new_video_clip)
                            {
                                // video clip is insert into track, we need check if this track has linked track
                                if (track && track->mLinkedTrack != -1)
                                {
                                    MediaTrack * relative_track = timeline->FindTrackByID(track->mLinkedTrack);
                                    if (relative_track && relative_track->mType == MEDIA_AUDIO)
                                    {
                                        if (new_video_clip->mGroupID == -1)
                                        {
                                            timeline->NewGroup(new_video_clip);
                                        }
                                        timeline->AddClipIntoGroup(new_audio_clip, new_video_clip->mGroupID);
                                        relative_track->InsertClip(new_audio_clip, mouseTime);
                                        timeline->Updata();
                                    }
                                    else
                                        create_new_track = true;
                                }
                                else
                                    create_new_track = true;
                            }
                            else
                            {
                                // no video stream
                                if (track && track->mType == MEDIA_AUDIO)
                                {
                                    // update clip info and push into track
                                    track->InsertClip(new_audio_clip, mouseTime);
                                    timeline->Updata();
                                }
                                else
                                {
                                    create_new_track = true;
                                }
                            }
                        }
                        if (create_new_track)
                        {
                            if (new_video_clip)
                            {
                                if (new_video_clip->mGroupID == -1)
                                {
                                    timeline->NewGroup(new_video_clip);
                                }
                                timeline->AddClipIntoGroup(new_audio_clip, new_video_clip->mGroupID);
                            }
                            MediaTrack * audio_track = new MediaTrack("", MEDIA_AUDIO, timeline);
                            audio_track->mExpanded = true;
                            audio_track->InsertClip(new_audio_clip, mouseTime);
                            if (video_track)
                            {
                                video_track->mLinkedTrack = audio_track->mID;
                                audio_track->mLinkedTrack = video_track->mID;
                            }
                            timeline->m_Tracks.push_back(audio_track);
                            timeline->Updata();
                        }
                    }
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    // handle expanded button event
    if (expanded)
    {
        bool overExpanded = ExpendButton(draw_list, ImVec2(canvas_pos.x + 2, canvas_pos.y + 2), !*expanded);
        if (overExpanded && io.MouseReleased[0])
            *expanded = !*expanded;
    }

    // handle delete event
    for (auto clip : delClipEntry)
    {
        auto track = timeline->FindTrackByClipID(clip);
        if (track)
            track->DeleteClip(clip);
        timeline->DeleteClip(clip);
    }
    if (delTrackEntry != -1)
    {
        timeline->DeleteTrack(delTrackEntry);
    }
    if (removeEmptyTrack)
    {
        for (auto iter = timeline->m_Tracks.begin(); iter != timeline->m_Tracks.end();)
        {
            if ((*iter)->m_Clips.size() <= 0)
            {
                auto track = *iter;
                iter = timeline->m_Tracks.erase(iter);
                delete track;
            }
            else
                ++iter;
        }
    }

    // handle group event
    if (groupClipEntry.size() > 0)
    {
        ClipGroup new_group;
        timeline->m_Groups.push_back(new_group);
        for (auto clip_id : groupClipEntry)
        {
            auto clip = timeline->FindClipByID(clip_id);
            timeline->AddClipIntoGroup(clip, new_group.mID);
        }
    }

    for (auto clip_id : unGroupClipEntry)
    {
        auto clip = timeline->FindClipByID(clip_id);
        timeline->DeleteClipFromGroup(clip, clip->mGroupID);
    }

    // handle track moving
    if (trackMovingEntry != -1 && trackEntry != -1 && trackMovingEntry != trackEntry)
    {
        timeline->MovingTrack(trackMovingEntry, trackEntry);
    }

    // for debug
    //ImGui::BeginTooltip();
    //ImGui::Text("%s", std::to_string(mouseEntry).c_str());
    //ImGui::Text("%s", std::to_string(clipMovingEntry).c_str());
    //ImGui::Text("%s", std::to_string(trackMovingEntry).c_str());
    //ImGui::Text("%s", std::to_string(trackEntry).c_str());
    //ImGui::Text("%s", MillisecToString(mouseTime).c_str());
    //ImGui::EndTooltip();
    // for debug end
    return ret;
}
} // namespace MediaTimeline/Main Timeline