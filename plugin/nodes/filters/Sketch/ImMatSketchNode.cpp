#include <UI.h>
#include <imgui_json.h>
#include <imgui_extra_widget.h>
#include <ImVulkanShader.h>
#include "Sketch_vulkan.h"
#define NODE_VERSION    0x01000000

namespace BluePrint
{
struct SketchNode final : Node
{
    BP_NODE_WITH_NAME(SketchNode, "Sketch", "CodeWin", NODE_VERSION, VERSION_BLUEPRINT_API, NodeType::External, NodeStyle::Default, "Filter#Video#Stylization")
    SketchNode(BP* blueprint): Node(blueprint) { m_Name = "Sketch"; m_HasCustomLayout = true; m_Skippable = true; }

    ~SketchNode()
    {
        if (m_filter) { delete m_filter; m_filter = nullptr; }
        ImGui::ImDestroyTexture(&m_logo);
    }

    void Reset(Context& context) override
    {
        Node::Reset(context);
        m_mutex.lock();
        m_MatOut.SetValue(ImGui::ImMat());
        m_mutex.unlock();
    }

    FlowPin Execute(Context& context, FlowPin& entryPoint, bool threading = false) override
    {
        auto mat_in = context.GetPinValue<ImGui::ImMat>(m_MatIn);
        if (m_IntensityIn.IsLinked()) m_intensity = context.GetPinValue<float>(m_IntensityIn);
        if (m_StepIn.IsLinked()) m_step = context.GetPinValue<int32_t>(m_StepIn);
        if (!mat_in.empty())
        {
            int gpu = mat_in.device == IM_DD_VULKAN ? mat_in.device_number : ImGui::get_default_gpu_index();
            if (!m_Enabled)
            {
                m_MatOut.SetValue(mat_in);
                return m_Exit;
            }
            if (!m_filter || gpu != m_device)
            {
                if (m_filter) { delete m_filter; m_filter = nullptr; }
                m_filter = new ImGui::Sketch_vulkan(gpu);
            }
            if (!m_filter)
            {
                return {};
            }
            m_device = gpu;
            ImGui::VkMat im_RGB; im_RGB.type = m_mat_data_type == IM_DT_UNDEFINED ? mat_in.type : m_mat_data_type;
            m_NodeTimeMs = m_filter->filter(mat_in, im_RGB, m_intensity, m_step);
            m_MatOut.SetValue(im_RGB);
        }
        return m_Exit;
    }

    void WasUnlinked(const Pin& receiver, const Pin& provider) override
    {
        if (receiver.m_ID == m_IntensityIn.m_ID)
        {
            m_IntensityIn.SetValue(m_intensity);
        }
        if (receiver.m_ID == m_StepIn.m_ID)
        {
            m_StepIn.SetValue(m_step);
        }
    }

    bool DrawSettingLayout(ImGuiContext * ctx) override
    {
        // Draw Setting
        auto changed = Node::DrawSettingLayout(ctx);
        ImGui::Separator();
        changed |= Node::DrawDataTypeSetting("Mat Type:", m_mat_data_type);
        return changed;
    }

    bool DrawCustomLayout(ImGuiContext * ctx, float zoom, ImVec2 origin, ImGui::ImCurveEdit::Curve * key, bool embedded) override
    {
        ImGui::SetCurrentContext(ctx);
        float setting_offset = 320;
        if (!embedded)
        {
            ImVec2 sub_window_pos = ImGui::GetCursorScreenPos();
            ImVec2 sub_window_size = ImGui::GetWindowSize();
            setting_offset = sub_window_size.x - 80;
        }
        bool changed = false;
        float _intensity = m_intensity;
        int _step = m_step;
        static ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Stick;
        ImGui::PushStyleColor(ImGuiCol_Button, 0);
        ImGui::PushItemWidth(200);
        ImGui::BeginDisabled(!m_Enabled || m_IntensityIn.IsLinked());
        ImGui::SliderFloat("Intensity##Sketch", &_intensity, 0.0f, 1.f, "%.2f", flags);
        ImGui::SameLine(setting_offset);  if (ImGui::Button(ICON_RESET "##reset_intensity##Sketch")) { _intensity = 0.4f; changed = true; }
        ImGui::ShowTooltipOnHover("Reset");
        ImGui::EndDisabled();
        ImGui::BeginDisabled(!m_Enabled);
        if (key) ImGui::ImCurveCheckEditKeyWithIDByDim("##add_curve_intensity##Sketch", key, ImGui::ImCurveEdit::DIM_X, m_IntensityIn.IsLinked(), "Intensity##Sketch@" + std::to_string(m_ID), 0.01f, 1.f, 0.4f, m_IntensityIn.m_ID);
        ImGui::EndDisabled();
        ImGui::BeginDisabled(!m_Enabled || m_StepIn.IsLinked());
        ImGui::SliderInt("Step##Sketch", &_step, 1, 5, "%d", flags);
        ImGui::SameLine(setting_offset);  if (ImGui::Button(ICON_RESET "##reset_step##Sketch")) { _step = 1; changed = true; }
        ImGui::ShowTooltipOnHover("Reset");
        ImGui::EndDisabled();
        ImGui::BeginDisabled(!m_Enabled);
        if (key) ImGui::ImCurveCheckEditKeyWithIDByDim("##add_curve_step##Sketch", key, ImGui::ImCurveEdit::DIM_X, m_StepIn.IsLinked(), "Step##Sketch@" + std::to_string(m_ID), 1, 4, 1, m_StepIn.m_ID);
        ImGui::EndDisabled();
        ImGui::PopItemWidth();
        ImGui::PopStyleColor();
        if (_intensity != m_intensity) { m_intensity = _intensity; changed = true; }
        if (_step != m_step) { m_step = _step; changed = true; }
        return m_Enabled ? changed : false;
    }

    int Load(const imgui_json::value& value) override
    {
        int ret = BP_ERR_NONE;
        if ((ret = Node::Load(value)) != BP_ERR_NONE)
            return ret;

        if (value.contains("mat_type"))
        {
            auto& val = value["mat_type"];
            if (val.is_number()) 
                m_mat_data_type = (ImDataType)val.get<imgui_json::number>();
        }
        if (value.contains("intensity"))
        {
            auto& val = value["intensity"];
            if (val.is_number()) 
                m_intensity = val.get<imgui_json::number>();
        }
        if (value.contains("step"))
        {
            auto& val = value["step"];
            if (val.is_number()) 
                m_step = val.get<imgui_json::number>();
        }
        return ret;
    }

    void Save(imgui_json::value& value, std::map<ID_TYPE, ID_TYPE> MapID) override
    {
        Node::Save(value, MapID);
        value["mat_type"] = imgui_json::number(m_mat_data_type);
        value["intensity"] = imgui_json::number(m_intensity);
        value["step"] = imgui_json::number(m_step);
    }

    void DrawNodeLogo(ImGuiContext * ctx, ImVec2 size, std::string logo) const override
    {
        // Node::DrawNodeLogo(ctx, size, std::string(u8"\ue746"));
        if (ctx) ImGui::SetCurrentContext(ctx); // External Node must set context
        if (!m_logo) m_logo = Node::LoadNodeLogo((void *)logo_data, logo_size);
        Node::DrawNodeLogo(m_logo, m_logo_index, logo_cols, logo_rows, size);
    }

    span<Pin*> GetInputPins() override { return m_InputPins; }
    span<Pin*> GetOutputPins() override { return m_OutputPins; }
    Pin* GetAutoLinkInputFlowPin() override { return &m_Enter; }
    Pin* GetAutoLinkOutputFlowPin() override { return &m_Exit; }
    vector<Pin*> GetAutoLinkInputDataPin() override { return {&m_MatIn}; }
    vector<Pin*> GetAutoLinkOutputDataPin() override { return {&m_MatOut}; }

    FlowPin   m_Enter   = { this, "Enter" };
    FlowPin   m_Exit    = { this, "Exit" };
    MatPin    m_MatIn   = { this, "In" };
    FloatPin  m_IntensityIn  = { this, "Intensity" };
    Int32Pin  m_StepIn = { this, "Step" };
    MatPin    m_MatOut  = { this, "Out" };

    Pin* m_InputPins[4] = { &m_Enter, &m_MatIn, &m_IntensityIn, &m_StepIn };
    Pin* m_OutputPins[2] = { &m_Exit, &m_MatOut };

private:
    ImDataType m_mat_data_type {IM_DT_UNDEFINED};
    int m_device            {-1};
    float m_intensity       {0.5f};
    int m_step              {1};
    ImGui::Sketch_vulkan * m_filter   {nullptr};
    mutable ImTextureID  m_logo {nullptr};
    mutable int m_logo_index {0};

    const unsigned int logo_width = 100;
    const unsigned int logo_height = 100;
    const unsigned int logo_cols = 1;
    const unsigned int logo_rows = 1;
    const unsigned int logo_size = 6279;
    const unsigned int logo_data[6280/4] =
{
    0xe0ffd8ff, 0x464a1000, 0x01004649, 0x01000001, 0x00000100, 0x8400dbff, 0x02020300, 0x03020203, 0x04030303, 0x05040303, 0x04050508, 0x070a0504, 
    0x0c080607, 0x0b0c0c0a, 0x0d0b0b0a, 0x0d10120e, 0x0b0e110e, 0x1016100b, 0x15141311, 0x0f0c1515, 0x14161817, 0x15141218, 0x04030114, 0x05040504, 
    0x09050509, 0x0d0b0d14, 0x14141414, 0x14141414, 0x14141414, 0x14141414, 0x14141414, 0x14141414, 0x14141414, 0x14141414, 0x14141414, 0x14141414, 
    0x14141414, 0x14141414, 0xc0ff1414, 0x00081100, 0x03640064, 0x02002201, 0x11030111, 0x01c4ff01, 0x010000a2, 0x01010105, 0x00010101, 0x00000000, 
    0x01000000, 0x05040302, 0x09080706, 0x00100b0a, 0x03030102, 0x05030402, 0x00040405, 0x017d0100, 0x04000302, 0x21120511, 0x13064131, 0x22076151, 
    0x81321471, 0x2308a191, 0x15c1b142, 0x24f0d152, 0x82726233, 0x17160a09, 0x251a1918, 0x29282726, 0x3635342a, 0x3a393837, 0x46454443, 0x4a494847, 
    0x56555453, 0x5a595857, 0x66656463, 0x6a696867, 0x76757473, 0x7a797877, 0x86858483, 0x8a898887, 0x95949392, 0x99989796, 0xa4a3a29a, 0xa8a7a6a5, 
    0xb3b2aaa9, 0xb7b6b5b4, 0xc2bab9b8, 0xc6c5c4c3, 0xcac9c8c7, 0xd5d4d3d2, 0xd9d8d7d6, 0xe3e2e1da, 0xe7e6e5e4, 0xf1eae9e8, 0xf5f4f3f2, 0xf9f8f7f6, 
    0x030001fa, 0x01010101, 0x01010101, 0x00000001, 0x01000000, 0x05040302, 0x09080706, 0x00110b0a, 0x04020102, 0x07040304, 0x00040405, 0x00770201, 
    0x11030201, 0x31210504, 0x51411206, 0x13716107, 0x08813222, 0xa1914214, 0x2309c1b1, 0x15f05233, 0x0ad17262, 0xe1342416, 0x1817f125, 0x27261a19, 
    0x352a2928, 0x39383736, 0x4544433a, 0x49484746, 0x5554534a, 0x59585756, 0x6564635a, 0x69686766, 0x7574736a, 0x79787776, 0x8483827a, 0x88878685, 
    0x93928a89, 0x97969594, 0xa29a9998, 0xa6a5a4a3, 0xaaa9a8a7, 0xb5b4b3b2, 0xb9b8b7b6, 0xc4c3c2ba, 0xc8c7c6c5, 0xd3d2cac9, 0xd7d6d5d4, 0xe2dad9d8, 
    0xe6e5e4e3, 0xeae9e8e7, 0xf5f4f3f2, 0xf9f8f7f6, 0x00dafffa, 0x0001030c, 0x11031102, 0xfd003f00, 0xd13e9940, 0xc43d8631, 0x8fc52e2f, 0x9f57801e, 
    0x7e99ad78, 0xb684e827, 0x1de3e059, 0xa863ef07, 0x46887f75, 0x7870894f, 0x48d6d15a, 0x6bdeeded, 0x4e702399, 0xc7298cf3, 0x55f3d443, 0xc9df223e, 
    0xc34bf163, 0x5aac20a2, 0x00ff9c12, 0x03a858d7, 0x470bafd1, 0x9d18c6a8, 0x9c0c038a, 0xd05a9fe3, 0xa8469089, 0xbfea958d, 0xcbacf3e7, 0x4d3d36af, 
    0xba0d455e, 0xed24e13f, 0xdc53edeb, 0xa3280bdf, 0x8da2e19e, 0x9c3cc309, 0x7282eb6c, 0xf73a707d, 0x0a060da0, 0x00124b8a, 0x02d420e7, 0x08ee3177, 
    0x725c31e0, 0x5db1e639, 0x4c59d4a5, 0x028231c3, 0x6241eec6, 0x00283c78, 0x859745d6, 0x4f404d34, 0x628dde1d, 0x9fc548f2, 0xc8e016ed, 0xa079f27d, 
    0xca22ce0e, 0x3c57905c, 0xfe76d8f3, 0xcbe293b5, 0xd7342f29, 0xbce4cb11, 0x17596388, 0xb9e641b6, 0x9ac3472d, 0x4d52818f, 0x3b97c247, 0xcf3235d6, 
    0x1ea9ebe1, 0x0290ae58, 0x33c4747f, 0x6de0f8c4, 0x57064e05, 0x7ca1bb8a, 0x56c68340, 0x607ff727, 0x4d152dd5, 0xa92391f6, 0x9d43d29e, 0x0186d7a1, 
    0x82a59282, 0x3a705eee, 0xc18f0e00, 0x1e6a52d7, 0xe096b31e, 0x85b17091, 0x78708c72, 0xa4473ca9, 0xf7e8ad4b, 0x18dc9016, 0x95e1e167, 0x204586bf, 
    0x1cd93ea9, 0xf0ca1afb, 0x7e56e4c5, 0x72bfb515, 0x97b4c5b1, 0x645c4062, 0x5b414f8e, 0xa49a4c90, 0x24722c0b, 0x2303acb6, 0x032b4344, 0x000d3ddf, 
    0x00ffdd78, 0xb5874f88, 0x1ce20bfd, 0xd5b1a23b, 0xc40f3d1a, 0xb673ca16, 0x9961e54d, 0x9014f147, 0x55ce3895, 0xea8a338e, 0x6bfc363e, 0xf80c3ed2, 
    0x46f15123, 0xb187f6af, 0x618164b3, 0x73cb4cb5, 0xc211f970, 0xf5e4a2a7, 0xd6318063, 0x6a29feb2, 0x51c30f5a, 0x213e6bad, 0xbac11378, 0xacc5a395, 
    0x46a23b45, 0x083b6c92, 0x2083422c, 0x6524001b, 0x5e93e38a, 0xf82ffb1d, 0x1eed3541, 0x103f8a3f, 0xd7b1baa5, 0x2d3468c0, 0x418c5e3f, 0xc5a5812e, 
    0x6b3caa32, 0x04ce3026, 0x9d23e67b, 0x8e0141e1, 0x0aaf0de0, 0x8bf8167c, 0xe7ca4af1, 0xf8223ec5, 0x061fe38f, 0xc3957a6a, 0x8307dacc, 0xc68ae12f, 
    0xc70270c5, 0x20c92497, 0x4227cdc7, 0x2703194f, 0x5b19fe6f, 0x5bf41fc5, 0x00ff2e7e, 0xb1d6e2e0, 0x6900ffee, 0xdc0f0f2b, 0x838715cd, 0x5fe25bbc, 
    0xe29a473a, 0x8f450f6d, 0x6ef31653, 0x495cd02a, 0xd7c92ee6, 0x3e1edd68, 0xffa25650, 0x3fb08600, 0x24fe4af4, 0x1be100ff, 0x1e00ff1f, 0x93510fa0, 
    0x0d9e76c1, 0x62802ef1, 0x6f2fae95, 0xddcded0d, 0xc8f272d3, 0xf6490edb, 0x173d5018, 0x99f84feb, 0x5a7c5024, 0x4798cff0, 0x9c54bb2a, 0x7c743e16, 
    0x0fbaca0f, 0x7e04c119, 0x92e7f022, 0x8ccb2ea4, 0x16f74f02, 0xd689bfb9, 0x73f1cdd1, 0x6fc0e1c2, 0x3c7965dd, 0xf12f3a1f, 0xd64c0fa0, 0x296d4bf5, 
    0x9bbabe65, 0x81b7b6ca, 0x0f4296e5, 0x9819aac8, 0x1300afe3, 0x3c84af5c, 0xc5c42755, 0xd1518fb9, 0xce8a4f1b, 0xcf099242, 0xc0a84002, 0x50567d10, 
    0x018d7f6a, 0xab26f0e0, 0xde23040c, 0x2d705a24, 0x3271dbc8, 0xecf8df40, 0x6add9ea7, 0x99166111, 0xcc6a3de2, 0x2e6fc423, 0x2015ec52, 0x47f116a7, 
    0xdd1f24df, 0x6e0328fd, 0x486beb4b, 0x4e3db52e, 0x685b3bee, 0x37f2ccf2, 0x79ee2110, 0x710027e3, 0x69654d92, 0x24bed777, 0x21a87bbc, 0x4ad34993, 
    0xcae48984, 0x439d4e5c, 0x6af51f2a, 0x986fc178, 0x431d408e, 0xe1bfd266, 0xbd47b826, 0x192e6856, 0xd42d2c2f, 0x8fe9da85, 0x9cebb9de, 0x301200e3, 
    0xd2e91979, 0xf1795d5a, 0x9a77b7c9, 0xe08cbc84, 0x3ab0e170, 0x02283d7e, 0x569c95ed, 0x48a0e5eb, 0x03e5fc43, 0xd4133216, 0xe4357992, 0xfe1a00ff, 
    0xc8a8cf15, 0xd5f031fe, 0xe39bdee5, 0x51ce322d, 0x54c82c6a, 0x247315d5, 0x1c033776, 0x03aa48a3, 0x010b9003, 0x8940bdd6, 0x1e297f61, 0xf393df37, 
    0x1ef20767, 0xad971cf5, 0xa4890b67, 0x81dbeadc, 0x719d708c, 0x7b8c73d4, 0x3d0fa0f4, 0x6dbdb6f0, 0xb3d9dfe2, 0xdb0856fb, 0x4b7af7ec, 0xbc4fb2cd, 
    0x619504b9, 0x479083d8, 0x08af0862, 0x3bbe83fd, 0xf0207e6b, 0x23e1af46, 0x8ef85fa3, 0x22c80f2f, 0x0b835b83, 0xbce1266d, 0x468ef9db, 0x692723e9, 
    0x92639c0b, 0x3ad1cf38, 0xf647803f, 0x0c93f67d, 0x37c3d3a2, 0x36972cab, 0xf22c3911, 0x374a4bca, 0xc50a9632, 0x8c613889, 0x6315b8fc, 0xf00bbec2, 
    0x098fc18f, 0xf012fee8, 0xb4d291f5, 0xb2b44168, 0x3ecb12b5, 0x05715bc0, 0xdc8899dd, 0x344f927a, 0xc3f5f301, 0x27b51f6a, 0xb53d01ed, 0x7ffabccd, 
    0xdf1bfe80, 0xecce9694, 0x5efb9685, 0x2833bc09, 0x4a8184dd, 0xdcc13894, 0xbe963e48, 0xf1a1ae24, 0x67bac753, 0x88346dc3, 0x3551adee, 0xf6ea135f, 
    0xc13e46f1, 0xb5207268, 0x6bfda75d, 0x9c7144b6, 0x0ef35515, 0x88388277, 0xf85a28fe, 0x18bfc167, 0x215c20fe, 0x07f14e9b, 0x74edae8f, 0x968c76eb, 
    0x154310bb, 0x8da07caa, 0x2933bc80, 0x8371cadc, 0x89be06c7, 0x113e9afd, 0xf03f7c5d, 0xe47a9754, 0x0de37789, 0x2ded7f6a, 0x3329f272, 0x2167d71c, 
    0x55552842, 0x60aa8c54, 0x9204c928, 0x770035dc, 0x2df0119e, 0x3f3c848f, 0x90e9a469, 0x5b59a5cb, 0xb2b523ae, 0xe499a86d, 0xc4bc8fe4, 0x491e4b92, 
    0x94fdb335, 0x51efe77f, 0xa7bf00ff, 0x41596bfc, 0xd27af25c, 0x0368eaf3, 0x015e3cce, 0x65947ef1, 0x33821ced, 0x91d1fd91, 0xc6c75ceb, 0x8b0f9619, 
    0xfe2c071e, 0x7e427958, 0xaeb86826, 0xbe34e287, 0x3a7c8b5f, 0xe9bf6df8, 0xf2333d23, 0x2b57f82f, 0x373fd2f1, 0x3e816fe2, 0x9d72116c, 0xc7de9fb3, 
    0xc7671d40, 0xf8b5e5d9, 0x41dfa95d, 0x99269d17, 0xb4a96773, 0xb6627260, 0x31798aba, 0xe26c00ff, 0x5e56f873, 0x882f7fab, 0x4bbf4b7c, 0xecd4b4b7, 
    0x1724611a, 0x9103e80a, 0xf905d60e, 0xa8bb0300, 0x8aeb00ea, 0x22423cef, 0xd81d0937, 0xdec2e984, 0xd7f32759, 0x618c1172, 0xb336c6b9, 0x17aff493, 
    0xdaa75ff8, 0xfee938f8, 0x74d4b61d, 0xa63bfce8, 0x4d1b49d8, 0x99138d34, 0xe0a5e19e, 0x488c0d31, 0x7aa60f06, 0xeaa50750, 0x81b6adfa, 0x6900ffa2, 
    0x670d17ea, 0x00ff0e6e, 0xbb947f36, 0xf46788e1, 0xe8c0c066, 0x78a5cf38, 0x3cbe8e3f, 0xa541bc09, 0xa84da4c7, 0xd3b5d858, 0x7caf3ecf, 0xda3c2d1e, 
    0x824bb243, 0x94910610, 0xae421189, 0x7288b41c, 0x6f5c0137, 0x782afe8b, 0xb792c7a7, 0xe8ecd689, 0x75836e7a, 0x7d1aeaa7, 0x92265b84, 0x2ab47f56, 
    0xfbf25589, 0x4fb765de, 0x8cca9d2c, 0xc5f614ee, 0xaf8efdf1, 0x44734de2, 0xef81a7f0, 0xf11ce17f, 0xdaa8c675, 0x12019fa7, 0x246163c7, 0xd6a1fd92, 
    0x12671950, 0x26f3563c, 0xcc330fec, 0x0020843c, 0xc4ef1a75, 0xdeda8c6f, 0x5778a4a9, 0xacaddac0, 0xe4a9965a, 0xd62c495c, 0x61475c71, 0x4bf02661, 
    0x677226c8, 0x9c0d2c5f, 0x4131779e, 0x7852df82, 0x74d44413, 0x43e90f7f, 0x54a317c4, 0x8ab7a3d7, 0x8cb5bb3b, 0xaa328f46, 0x74148cf9, 0x917a82cb, 
    0x4fc35f5c, 0x0bbfe802, 0x2dc692ec, 0x5a1db555, 0x8e15daee, 0x3cdd77f6, 0x1d93788a, 0x62606337, 0xed19967b, 0xdba79b5e, 0xf28ca5f9, 0x6aa32816, 
    0x0f8c8111, 0xf2e48e98, 0x0b0ea079, 0x2d1adfc4, 0x433d356d, 0x9eb6f046, 0x1be2437c, 0xeab40d39, 0x2d6df224, 0x8e7023a7, 0xbf31826b, 0xdcc44e6e, 
    0xd78cf3dc, 0x7c101fcf, 0x551ff157, 0x11be8ef8, 0xb52cd5b0, 0xa59a8a3f, 0x4397fd8b, 0xd3a319d3, 0xb06d313c, 0xf485f6af, 0x26337383, 0x7ec6e5f3, 
    0x7d6c95f3, 0x2f7e74d0, 0x0bdff7eb, 0x0f6d00ff, 0x49095903, 0xe78cc7f0, 0x44ab2b8a, 0xa68b6293, 0x28943406, 0x8b707b3f, 0x3d47f69f, 0xbce0232b, 
    0x2fc3077a, 0xc4eba2d9, 0x2e5d5a92, 0x2dcfaeb1, 0x39a9bfd6, 0x20727161, 0x69b66b67, 0x7900ff19, 0x2baab280, 0xeac0e51c, 0xfc68004d, 0xad15f019, 
    0x067fc4ef, 0xedcd42f8, 0xc3c7f0e6, 0x523e3a1d, 0x6a5c2c0f, 0x792119ae, 0x88bb4a30, 0x8f58ee95, 0xf320b9e3, 0x082a695f, 0x940f5095, 0x075e310e, 
    0x1b782bfb, 0xc3eff050, 0xd5553fcd, 0x48f84fa3, 0xa6fa3275, 0xcd05c4a2, 0x1defc7c5, 0x880d0078, 0x061d8f62, 0x05f78ad3, 0x2e0076d5, 0xaf84e42d, 
    0x67988f04, 0xd0008afc, 0xc893ad50, 0x7669dcf6, 0x00ff560f, 0x8656cdbe, 0xd90c37f2, 0x73e30950, 0xff14f4e0, 0xeb47b500, 0x14fcfd1f, 0x147fe401, 
    0x3abec2f5, 0x9997a5f0, 0x75edca6e, 0x7cf63ef7, 0x43b5dea0, 0xfe9150e3, 0xa81cf826, 0x3974b641, 0xb9fe8163, 0x0fb9c23f, 0x695cd38a, 0x07bc1bbf, 
    0xfcd9fb61, 0x5a19b5d8, 0xc0445f46, 0xd11576fa, 0x4bb3d07e, 0x02afe36d, 0x65667ef9, 0x22338ebc, 0x6430c60d, 0xf11d9b5f, 0x6700c5e9, 0xd3f8657c, 
    0xa23ae8a6, 0xed8378df, 0xac84371a, 0xad379b22, 0xca953a97, 0x82639d18, 0x1fcfcb04, 0x8390842e, 0xc1310079, 0xebdacecf, 0xbe173fea, 0xfd1a6a23, 
    0xa26f93ce, 0xb2c59aea, 0x0d69306b, 0xe1596464, 0x6b81258c, 0xfcd9988b, 0x910bddc6, 0x91808f10, 0xa5947f31, 0x1dc5ef7a, 0x9df14a07, 0x91ead2fd, 
    0x71934619, 0xbeb01d7b, 0x20785b02, 0x4f063486, 0x96918db3, 0x4b0ae569, 0x49460b70, 0x3cafc066, 0x3fe2d7f8, 0xf62b780b, 0x87b4bf82, 0x5ac2975b, 
    0x50ccb58d, 0x45dec325, 0x940e9f06, 0x63c87ada, 0x003b2686, 0x5b2cb66b, 0x83b4cb68, 0x3880e239, 0xf137897f, 0xea780c8f, 0x6bb159db, 0x136aa131, 
    0xa6497369, 0xd63422dd, 0x636b2cf1, 0x7685121c, 0x7b9bf48e, 0xb7bc9168, 0x28329742, 0xbecc8011, 0x4747f0ef, 0xa027bcd4, 0x650de2cd, 0x3bf1a746, 
    0xa92c30c6, 0x71b41c68, 0x0ae97624, 0x4b9773b1, 0x2457d578, 0x42ecdcb1, 0x3c5f0bf5, 0xe3993e47, 0xcb9389bf, 0x0f79737f, 0x631b7c80, 0xd436fe68, 
    0x0d5b44e4, 0x8cb6535b, 0x9633722c, 0xcd0cb72a, 0x02f26d28, 0xae982dee, 0x6fe8e3e5, 0xd75023d9, 0xeaa5773c, 0x1df13b5e, 0xdab7b6b4, 0x3b7d92c0, 
    0x0b36964a, 0xf22c59a2, 0x2b319642, 0x481eb701, 0x9c1bc067, 0x6020d992, 0x63f8ab02, 0x9788d7e2, 0x8b4f5ac5, 0x9fbcf0e3, 0x2d92b06d, 0x324d7b32, 
    0x912ce4ea, 0x5d08adca, 0xb118e2f2, 0x643c75e6, 0x5f7dbe63, 0x6fe2efc1, 0xba37fc0d, 0x34fba2d1, 0x74d221fe, 0xb8065218, 0x865b600e, 0x54ac4d86, 
    0xe82a4c36, 0xdb60c506, 0x9a67034e, 0xe4a88bf4, 0x93ba4a87, 0x8d64eaec, 0xf2881209, 0x57d071db, 0x5aaf6917, 0x5e8de247, 0xb9b4bc35, 0xe9fef067, 
    0x5d608e21, 0x2b6c5664, 0xbc61782e, 0xc507508c, 0xf5d2357e, 0x06e22b1d, 0xb1ae7393, 0xadb46fd5, 0x3fd74857, 0x75b725b6, 0x63299266, 0x7923e86b, 
    0x5660984f, 0x3010c278, 0x324e6e7e, 0xdc108f7b, 0xe85be2c7, 0x20c01abe, 0x593c6e48, 0x0b14a67d, 0xedcf8e96, 0xc8dedff3, 0x1259c2bf, 0x0686783b, 
    0x0741ca36, 0xd1feaf22, 0xa893d6fa, 0x3ac7dff8, 0xba05c972, 0xf6e15f78, 0x8438a9a5, 0x34de1cc6, 0x1ea3d088, 0x8d9647aa, 0x1841bc8f, 0xc1afd6e0, 
    0xafc9021f, 0x117fd87e, 0xcf8866af, 0x2e32e9a1, 0xa93ba7af, 0x4ecdfc68, 0x0249dede, 0xb0fb0836, 0x23e9c191, 0x07d0787f, 0xaa45bad8, 0x04a458da, 
    0xb825a860, 0x88f926fe, 0xa8e20cfa, 0xf6c48778, 0x55851c7e, 0x779ea4d1, 0x1a6fc131, 0x195b7916, 0xf5e4a8c2, 0x6bf20cfa, 0x16933f58, 0x013260f7, 
    0x573cf53c, 0x0f88e11f, 0x752eaf88, 0x43fbb4cb, 0xe9f170c9, 0x92bbccf1, 0x7c3b15da, 0x72de1fc5, 0x603bdc09, 0x2c800267, 0x7dbba65b, 0x48b49910, 
    0xb1fb812d, 0x801576c9, 0x801490ed, 0x4f6ab27d, 0xf47febb0, 0x00ffb50c, 0xc2fedfc0, 0xcaeda8ba, 0xf7160bae, 0x94fe2067, 0x7ffb24ef, 0x1f406ddf, 
    0xe96bdd38, 0xd066fca9, 0xbc5dfb22, 0x936958b3, 0x9133e2ac, 0x60db9119, 0x4ada6823, 0x380f4ec4, 0x40fbad35, 0xe37365c1, 0x5e5b012f, 0xc8e9706a, 
    0x678868f7, 0x793b6157, 0x287f04aa, 0xcbc9413b, 0x5eeb011c, 0x88bfe115, 0xfe85ef5a, 0x7eb6ea3a, 0x68faf030, 0x46db1a37, 0x106bc5f0, 0xd4829611, 
    0x138498a7, 0x775459b9, 0xaad72438, 0xc1f2d87e, 0x5e5669a5, 0x426d9e20, 0x2e48f408, 0x96e9c32f, 0x63c84ed1, 0xe6c9f705, 0xa70d0210, 0xb2e70c85, 
    0xf3001a9c, 0xb5896bfb, 0x634714cf, 0x6791b612, 0xa13ea736, 0x4755ebf6, 0x8f7bbb7b, 0x70c3ddb7, 0xb1921c50, 0x63101941, 0xe74002cb, 0xe2671596, 
    0xf885c257, 0x93066db7, 0x3a7769ab, 0x0bd4fd75, 0x25ae7369, 0x7bcbdbb2, 0xebb49262, 0x92cbd160, 0x7d529d08, 0xa34264dc, 0xac618deb, 0x57fa6b5c, 
    0x0d45fc82, 0x151efc8d, 0x8d1b4bb8, 0xb6b5554b, 0x5a379364, 0x7878ba54, 0xec933800, 0x36f916ef, 0x99d91572, 0xb4ee9a42, 0xe4f5d5d6, 0x9b7bca8a, 
    0x36c6538d, 0xbe676f2a, 0xd59a2fd1, 0x30c6e2fc, 0x73a54da6, 0x385779f6, 0xee1c635c, 0x162b0fa0, 0x3517df17, 0xf4d2bbb4, 0x053f3b6b, 0x0bea5a78, 
    0x0ad32298, 0xa8736745, 0xf9124fdb, 0xd87c0a03, 0xeedc95ad, 0x25ee1552, 0x18cbdc66, 0xc217f415, 0x083f42bd, 0x1a7e176b, 0x0d2d6fba, 0xf26daad6, 
    0x53eae000, 0x677b88ca, 0x513fe36c, 0x5bf0535e, 0xfc095f55, 0xd5f0b13d, 0x266f0bcc, 0x52c36683, 0xcb13add3, 0xddf236b7, 0xc413acd9, 0x6289cd0f, 
    0xe6bb9312, 0x480eae0e, 0x31bee8c9, 0xbe3d6dcd, 0xdcd9a399, 0x71e26bc9, 0xd5a0157d, 0x6f4dcae3, 0xcd11ed23, 0x21b2e185, 0xdcf09401, 0x07d07af2, 
    0xbf86dad0, 0x722c5aa9, 0x4bdb5bd8, 0xc125f369, 0x63c698ba, 0xe5c330c6, 0xe54f8e20, 0x78ac735e, 0x9ec1e3b6, 0x8837f11f, 0x33746cae, 0x7945d8a6, 
    0xdb5cdfa9, 0x3cdc2bcb, 0xc694ee11, 0x905645b9, 0x9ca97da9, 0xf0b66b65, 0x6df586ef, 0xea104a2f, 0x227ed4fa, 0x475e8030, 0x857148b0, 0x7bcccfc0, 
    0xf31a7f82, 0xe11fda2f, 0x33fe8807, 0x7e4c9ae8, 0xdd74d51e, 0xf46b3431, 0xf5b23fd4, 0x3efb670b, 0x05a3d1a6, 0x60a395b7, 0x53c93756, 0xda5bf9c1, 
    0xe1af3c80, 0xbf8bc34f, 0xc52f7810, 0x6ec6ad6b, 0x6de349bc, 0x19b56f11, 0x8ad4c6cd, 0xaa8deb4b, 0x241acb9e, 0x1f04e351, 0x45709024, 0xabecc77b, 
    0xc13fc5a7, 0x7beb0bdf, 0xbaeadf02, 0xa46e9e26, 0xe299b9dd, 0x93018eb5, 0xcf6c00ff, 0xbfe735d3, 0x0d213e09, 0x8ca7e26f, 0x78178f34, 0x78026f66, 
    0x9b346ca1, 0x893b3468, 0xb667e5d6, 0x3533ae85, 0x4fb4cba4, 0x2c182d11, 0x02582a00, 0xdff60407, 0xcff5ed01, 0xf020fc0d, 0xac6df65f, 0x686bba17, 
    0x80aad296, 0x7c5f5256, 0x12c0769c, 0x5d0fa028, 0xe9ea1df1, 0x71a5065a, 0xbc5a5bd0, 0x81032798, 0x6be0af9a, 0x37f8b37f, 0x9b51c949, 0x808eb338, 
    0xded7076d, 0x67e2dbbc, 0xe075b1c4, 0xe3d27e3d, 0xaed3bb4e, 0x4831b424, 0x44919bce, 0x0882b08e, 0x1fd7b9ef, 0x28fee85a, 0xfed1527c, 0x0b267807, 
    0xc425f1ab, 0xd3b294f2, 0x3254bb34, 0xb44fdf5c, 0x1c248a04, 0xe8731cb3, 0xe8013439, 0x90715542, 0xec200f32, 0xddbfdea5, 0x2bc700ff, 0x5d079de5, 
    0x7b7ca1fd, 0x2fb926a7, 0xe01d7c8c, 0xe225ba15, 0xfa904ed0, 0xfcc7db8b, 0x8012e73b, 0x0c5c47cc, 0x5a73647a, 0xb47fd91f, 0x4f16fd07, 0x28e17f07, 
    0x7efc00ff, 0xb37c3e80, 0x7ca885f8, 0xfb21f844, 0x52e3f839, 0xaed5bc65, 0x5b1bb5ae, 0xcee5c7bb, 0x3fc0a62d, 0x2bfa0a8a, 0x62d4c9f6, 0x2bfcd2b5, 
    0x0bd77067, 0x5bac7f65, 0xc8d0cc5b, 0x9808d863, 0xd88d3c17, 0xe6bcfc04, 0xf8432bbc, 0x0bf8a94b, 0xb4d068f6, 0xdbfe4e3d, 0x3cd0bd50, 0x7be2745d, 
    0x29de084c, 0xa89495a0, 0x00ff3e1d, 0xfbfd5eeb, 0x78dbd854, 0x475ff888, 0x782b9ebb, 0x4632e7ee, 0xde5f38f2, 0xc8b8d829, 0xf4d4f33f, 0xac9f0fa0, 
    0x0fe381f4, 0x7f0b5d1d, 0xdb0fefec, 0xa4a5baea, 0x33c63156, 0xe98ab50d, 0x9307b2d7, 0x7cfedd66, 0x2a1214a9, 0x8614ce7c, 0xf45a8821, 0x32be86bf, 
    0x6c2fd7bc, 0xdbf4d1a3, 0xf2fa3afd, 0xd5a54be6, 0x7141b5ae, 0x0b8bb61c, 0x0ced5746, 0x53f37709, 0x5e454f1b, 0x83ae1998, 0x862ffb52, 0xe3da8ee5, 
    0x5e93b951, 0x1abb46ba, 0x43fbb885, 0x43dc9134, 0x0daa036f, 0xe689c0b1, 0xa4234504, 0xaf39c0b9, 0x5e24f024, 0xa90ef120, 0x7586a7a6, 0xdec177ed, 
    0x9244d41a, 0x5cdad2e8, 0x1bf2b2fd, 0xc3b20ea5, 0xa150a03d, 0x90b82319, 0x19eba032, 0x01d49223, 0x782fc36b, 0x0df12afe, 0xc5836af2, 0xc5e1c551, 
    0x342fd4b6, 0xe6f652d7, 0xa86b4a3d, 0xac3bd044, 0x2bb6228a, 0x0ca75d11, 0x79eb3d0f, 0x00ff8bf5, 0x24fed688, 0xda1076f1, 0x449ff85f, 0x225974d3, 
    0x595b9653, 0x714c6546, 0xce216387, 0xe3d51e4f, 0x1a973697, 0xc33d83be, 0xa879c3db, 0x7636cbf8, 0xb4ed959a, 0x69d9db70, 0x6c4933f6, 0xf9711ed7, 
    0xa50a569a, 0x6472809d, 0xbe1bd353, 0xe15df8a3, 0xea145feb, 0x7cd316fe, 0x876f013f, 0xcd8b0ebe, 0x4a78528b, 0x79a8aba6, 0x4b985a7e, 0xc1285e00, 
    0x3300bf73, 0x34031937, 0x45d1ef01, 0x8a81ad15, 0x2329d438, 0xf1581d71, 0x4e4f7d92, 0x38a29c6a, 0x930a6049, 0xab7d128c, 0x11dd4462, 0x19c0718d, 
    0xb2067e00, 0x6c6a17ef, 0xbcfb066c, 0x03a81df7, 0x6d00ffc2, 0x9e97870f, 0xaa25f827, 0x52c39778, 0x5fe365cb, 0xb9264108, 0x12dadfa2, 0xa9d14693, 
    0xb80ce1f3, 0x7a8b253b, 0xaf78e895, 0x605ff85f, 0xde865f74, 0x4850b213, 0xe2acd1b5, 0xd806f000, 0xf71f4011, 0x79db6acd, 0x5b539657, 0x6e2c8b4e, 
    0x0c23720c, 0x02c35386, 0xf335083d, 0x3e6e86a7, 0xc16b7823, 0x1ee8cb1a, 0xd4508f22, 0x5da83ffc, 0x663a5769, 0x15a66aa9, 0x0860b610, 0x422b1846, 
    0x50ab9fd0, 0xe95eae07, 0x1e7fe32f, 0x38ddd85c, 0xc5d24787, 0x225bc3b5, 0xd2cedc8c, 0xffc45d17, 0x6af90a00, 0xab7dea71, 0x0f56d487, 0x75183f88, 
    0xc4de120f, 0x862f1a97, 0xd3b064ad, 0xde5f4656, 0x325cdc4e, 0xb1b2dd03, 0xbcc31f06, 0x483fae77, 0x5f74eaf8, 0x113fb686, 0x2ea75d7c, 0x86e7e28d, 
    0xc38f4ef7, 0xdcd24a16, 0x86a4de5d, 0x4b468d34, 0x406e393f, 0xbf5a0d55, 0x436db507, 0x877ff046, 0x7214b306, 0x95528b78, 0x42b0b5a5, 0x8b19f906, 
    0x314f4e19, 0xfe0192c7, 0x01b95d22, 0xedd90368, 0xe3d3b026, 0x1b595f30, 0xcc20d5bb, 0x45eedbf0, 0x39d80162, 0x30300a18, 0xfb9f6a07, 0x00ff454f, 
    0xf8bf44a0, 0xd900ff09, 0x5aea9257, 0x6ffdc4d7, 0xe13fd457, 0x83b1ba02, 0x1b73d6c3, 0x4850f345, 0xe6012acf, 0xe4092c31, 0xdf06959e, 0x82fe0fda, 
    0x03fe6f3a, 0x1300ff27, 0xe0631e40, 0xbedd36ed, 0x8d53fc15, 0xdf349151, 0xc22c8815, 0x8cb8dfc8, 0x9c2a4024, 0x1f7618e7, 0xdbfe688d, 0x4d7cd32e, 
    0xc4642df8, 0x516d9efd, 0x7d24a837, 0xfd04e2e9, 0xf8abde71, 0x7f49fe2f, 0x1cfb5fc6, 0x2a58f63f, 0xfe53db7f, 0xffc0ef4a, 0x430afb00, 0x31a500ff, 
    0xe36107d0, 0x4f3d1a7d, 0xe296b351, 0x299a867b, 0x55d2d8b4, 0x5de26393, 0xb06c157a, 0x480b6cc1, 0x329018b0, 0x1e73bcfe, 0xaf6bbe36, 0xf823fe85, 
    0x0741e5a1, 0xd8db748c, 0xcb1b9bb4, 0xda228468, 0x142c2102, 0x9d4f322e, 0xc4b32528, 0xed187684, 0x00ff4ffc, 0xd77f701f, 0x00ff8dce, 0x01d703a9, 
    0x34f9dfe2, 0xd87f1acf, 0x06fd0f7e, 0xfe3680d6, 0xf000fc0a, 0x0bf19386, 0x5b5cf2c5, 0xa46f6b5c, 0xe958336a, 0x7df3dabf, 0x64c059a0, 0xa4c6e5df, 
    0xb95d4661, 0x7d4d7dc6, 0x71b41d55, 0x84221a43, 0x00049458, 0x395e0160, 0x47febff0, 0xf6bf88df, 0xa5f9931c, 0x1ff0477b, 0x0094fea0, 0x3d6606d8, 
    0x45fdb07a, 0x18027f47, 0x816424e4, 0x6aec9391, 0xc7f57f5b, 0xfb00ffea, 0x00ff252d, 0x9f7ea9fa, 0x5f06d0d0, 0xe5ce9190, 0x8e1f3846, 0xe9b4c76b, 
    0xfbf84f1b, 0xf958bbe3, 0x4a8bbbb5, 0xe50792bc, 0xe385e63c, 0x2bb10367, 0x00ff791c, 0xbfb25774, 0x0ffae7fa, 0x9f305ee9, 0x2e1e71f2, 0x0daf00ff, 
    0x4100ff23, 0x0f3a809e, 0xc30bf819, 0x586757d6, 0x9a9c451a, 0xddc69784, 0x315478a6, 0x1298bb42, 0x03313f00, 0x4e5cf33c, 0x00ff6fb9, 0xf189c608, 
    0x92d6c607, 0xb220fe48, 0xd6922dd2, 0x8178db79, 0xa0b25c65, 0x3b003aec, 0xf5f91e05, 0x00ff1f5f, 0xec7f31c8, 0xb400ff2b, 0x8f7f79cd, 0xfc90e4bf, 
    0xb000ff4d, 0x00ff9f56, 0x6000c5a2, 0x0a1fcefe, 0x7c27fe2d, 0xf5d5b733, 0xd6eb104f, 0x8f6602f7, 0x68cc9565, 0xc4c8aa9c, 0x2d4932e6, 0xef491223, 
    0x00ffa35e, 0xffa2c90c, 0xe2d1d000, 0x8b0dfc9f, 0x568d00ff, 0xff57ec3f, 0x0f522400, 0xfa3ffafa, 0x9af72a26, 0x00d9ff00, 
};
};
} // namespace BluePrint

BP_NODE_DYNAMIC_WITH_NAME(SketchNode, "Sketch", "CodeWin", NODE_VERSION, VERSION_BLUEPRINT_API, BluePrint::NodeType::External, BluePrint::NodeStyle::Default, "Filter#Video#Stylization")