#include <cstdint>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <array>
#include <vector>
#include <bx/file.h>
#include <bimg/bimg.h>
#include <bgfx/bgfx.h>


struct Mesh
{
    bgfx::VertexBufferHandle    vbh {BGFX_INVALID_HANDLE};
    bgfx::IndexBufferHandle     ibh {BGFX_INVALID_HANDLE};
    std::vector<float>          pos {};
    std::vector<std::uint32_t>  ind {};

    void create(std::vector<float> positions, std::vector<std::uint32_t> indices)
    {
        pos = std::move(positions);
        ind = std::move(indices);

        auto layout = bgfx::VertexLayout{};
        layout.begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .end();

        {
            const auto size = std::uint32_t(pos.size() * 4);
            vbh = bgfx::createVertexBuffer(bgfx::makeRef(pos.data(), size), layout);
        }

        {
            const auto size = std::uint32_t(ind.size() * 4);
            ibh = bgfx::createIndexBuffer(bgfx::makeRef(ind.data(), size), BGFX_BUFFER_INDEX32);
        }
    }

    void destroy()
    {
        if (bgfx::isValid(vbh)) bgfx::destroy(vbh);
        if (bgfx::isValid(ibh)) bgfx::destroy(ibh);
    }

    static Mesh box(float size)
    {
        auto vertices = std::vector<float> {
            -size,  size,
             size,  size,
             size, -size,
            -size, -size
        };

        auto indices = std::vector<std::uint32_t> {
            0, 1, 2,
            0, 2, 3
        };

        auto result = Mesh();
        result.create(std::move(vertices), std::move(indices));

        return result;
    }
};


struct FrameBuffers
{
    bgfx::FrameBufferHandle     scene       {BGFX_INVALID_HANDLE};
    bgfx::FrameBufferHandle     dummy       {BGFX_INVALID_HANDLE};
    bgfx::TextureHandle         readback    {BGFX_INVALID_HANDLE};

    void create()
    {
        scene = bgfx::createFrameBuffer(1024, 1024, bgfx::TextureFormat::RGBA8, std::uint64_t{0}
                                                                                | BGFX_TEXTURE_RT_MSAA_X16
                                                                                | BGFX_SAMPLER_MIN_POINT
                                                                                | BGFX_SAMPLER_MAG_POINT
                                                                                | BGFX_SAMPLER_U_CLAMP
                                                                                | BGFX_SAMPLER_V_CLAMP
        );

        dummy = bgfx::createFrameBuffer(1, 1, bgfx::TextureFormat::RGBA8, std::uint64_t{0}
            | BGFX_TEXTURE_RT_MSAA_X16
            | BGFX_SAMPLER_MIN_POINT
            | BGFX_SAMPLER_MAG_POINT
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP
        );

        readback = bgfx::createTexture2D(1024, 1024, false, 1, bgfx::TextureFormat::RGBA8, std::uint64_t{0}
            | BGFX_TEXTURE_BLIT_DST
            | BGFX_TEXTURE_READ_BACK
        );
    }

    void destroy()
    {
        bgfx::destroy(scene);
        bgfx::destroy(readback);
        bgfx::destroy(dummy);
    }
};


struct Program
{
    bgfx::ProgramHandle handle {BGFX_INVALID_HANDLE};
    bgfx::UniformHandle color {BGFX_INVALID_HANDLE};

    void create()
    {
        const auto load = [](std::string_view name, const std::string& type)
        {
            const auto path = std::filesystem::path(ROOTDIR) / "shaders" / name / (type + ".bin");
            const auto size = std::filesystem::file_size(path);
            auto* mem = bgfx::alloc(size);

            auto stream = std::ifstream(path.string(), std::ios::binary);
            stream.read(reinterpret_cast<char*>(mem->data), std::streamsize(size));

            return bgfx::createShader(mem);
        };

        auto vert = load("mesh", "vert");
        bgfx::setName(vert, "mesh:vert");

        auto frag = load("mesh", "frag");
        bgfx::setName(vert, "mesh:frag");

        handle = bgfx::createProgram(vert, frag, true);
        color = bgfx::createUniform("uMeshColor", bgfx::UniformType::Vec4, 1);
    }

    void destroy()
    {
        bgfx::destroy(color);
    }
};


using ColorI = std::array<std::uint8_t,4>;
using ColorF = std::array<float,4>;


struct util
{
    static std::uint32_t packColor(ColorI color)
{
    const auto R = std::uint32_t(color[0]) << 24;
    const auto G = std::uint32_t(color[1]) << 16;
    const auto B = std::uint32_t(color[2]) << 8;
    const auto A = std::uint32_t(color[3]);
    return R | G | B | A;
}

    static ColorF floatColor(ColorI colorI)
    {
        return {
            float(colorI[0]) / 255.0f,
            float(colorI[1]) / 255.0f,
            float(colorI[2]) / 255.0f,
            float(colorI[3]) / 255.0f
        };
    }
};


#define BLACK   ColorI{  0,  0,  0,255}
#define WHITE   ColorI{255,255,255,255}
#define GREEN   ColorI{  0,255,  0,255}


#define VIEW_SCENE      0
#define VIEW_READBACK   1


auto FBO    = FrameBuffers {};
auto PROG   = Program {};
auto MESH_A = Mesh {};
auto MESH_B = Mesh {};


void save(std::string fileName)
{
    bgfx::blit(
        VIEW_READBACK,
        FBO.readback, 0, 0,
        bgfx::getTexture(FBO.scene),
        0, 0, 1024, 1024
    );

    auto rawImage = std::vector<std::byte> {};
    rawImage.resize(1024 * 1024 * 4);

    auto pngImage = std::vector<std::byte> {};
    pngImage.resize(1024 * 1024 * 4);

    const auto readyFrame = bgfx::readTexture(FBO.readback, rawImage.data());
    while (bgfx::frame() < readyFrame) {}

    const auto path = (std::filesystem::path(ROOTDIR) / fileName).string();
    const auto filePath = bx::FilePath{path.c_str()};

    auto writer = bx::FileWriter {};

	bx::open(&writer, filePath, false, nullptr);
    bimg::imageWritePng(&writer, 1024, 1024, 1024 * 4, rawImage.data(), bimg::TextureFormat::RGBA8, false, nullptr);
    bx::close(&writer);
}


void draw(Mesh& mesh, ColorI clearColor, ColorI geoColor)
{
    const auto clear = std::uint16_t{0}
        | BGFX_CLEAR_COLOR
        | BGFX_CLEAR_DISCARD_DEPTH
        | BGFX_CLEAR_DISCARD_STENCIL;

    const auto state = std::uint16_t{0}
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_MSAA;

    bgfx::setViewFrameBuffer(VIEW_SCENE, FBO.scene);
    bgfx::setViewRect(VIEW_SCENE, 0, 0, 1024, 1024);
    bgfx::setViewMode(VIEW_SCENE, bgfx::ViewMode::Sequential);
    bgfx::setViewClear(VIEW_SCENE, clear, util::packColor(clearColor), 1.0f, 0);
    bgfx::setState(state);

    const auto floatMeshColor = util::floatColor(geoColor);

    bgfx::setUniform(PROG.color, floatMeshColor.data(), 1);
    bgfx::setVertexBuffer(0, mesh.vbh);
    bgfx::setIndexBuffer(mesh.ibh, 0, mesh.ind.size());
    bgfx::submit(VIEW_SCENE, PROG.handle);
}


int main(int argc, char** argv)
{
    auto initConfig = bgfx::Init{};
    initConfig.type = bgfx::RendererType::Vulkan;
    bgfx::init(initConfig);

    FBO.create();
    PROG.create();

    MESH_A = Mesh::box(0.5);
    MESH_B = Mesh::box(0.25);

    bgfx::touch(VIEW_SCENE);
    bgfx::touch(VIEW_READBACK);
    bgfx::setViewFrameBuffer(VIEW_READBACK, FBO.dummy);

    draw(MESH_A, BLACK, WHITE);
    draw(MESH_B, BLACK, GREEN);

    save("image.png");

    FBO.destroy();
    MESH_A.destroy();
    MESH_B.destroy();
    PROG.destroy();

    bgfx::shutdown();

    return 0;
}