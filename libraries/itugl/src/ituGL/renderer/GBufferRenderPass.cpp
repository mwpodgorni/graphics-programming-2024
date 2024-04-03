#include <ituGL/renderer/GBufferRenderPass.h>

#include <ituGL/camera/Camera.h>
#include <ituGL/shader/Material.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/renderer/Renderer.h>

GBufferRenderPass::GBufferRenderPass(int width, int height, int drawcallCollectionIndex)
    : m_drawcallCollectionIndex(drawcallCollectionIndex)
{
    InitTextures(width, height);
    InitFramebuffer();
}

void GBufferRenderPass::InitFramebuffer()
{
    m_framebuffer.Bind();

    m_framebuffer.SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Depth, *m_depthTexture);

    // (todo) 07.2: Set the albedo texture as color attachment 0

    // (todo) 07.3: Set the normal texture as color attachment 1

    // (todo) 07.5: Set the others texture as color attachment 2


    // (todo) 07.2: Set the draw buffers used by the framebuffer (all attachments except depth)
    m_framebuffer.SetDrawBuffers(std::array<FramebufferObject::Attachment, 3>(
        {
            FramebufferObject::Attachment::Color0,
            FramebufferObject::Attachment::Color1,
            FramebufferObject::Attachment::Color2
        }));

    FramebufferObject::Unbind();
}

void GBufferRenderPass::InitTextures(int width, int height)
{
    // (todo) 07.4: Depth: Set the min and magfilter as nearest
    m_depthTexture = std::make_shared<Texture2DObject>();
    m_depthTexture->Bind();
    m_depthTexture->SetImage(0, width, height, TextureObject::FormatDepth, TextureObject::InternalFormatDepth);


    // (todo) 07.2: Albedo: Bind the newly created texture, set the image, and the min and magfilter as nearest
    m_albedoTexture = std::make_shared<Texture2DObject>();


    // (todo) 07.3: Normal: Bind the newly created texture, set the image and the min and magfilter as nearest
    m_normalTexture = std::make_shared<Texture2DObject>();


    // (todo) 07.5: Others: Bind the newly created texture, set the image and the min and magfilter as nearest
    m_othersTexture = std::make_shared<Texture2DObject>();


    Texture2DObject::Unbind();
}

void GBufferRenderPass::Render()
{
    Renderer& renderer = GetRenderer();

    const Camera& camera = renderer.GetCurrentCamera();
    const auto& lights = renderer.GetLights();
    const auto& drawcallCollection = renderer.GetDrawcalls(m_drawcallCollectionIndex);

    // Bind the framebuffer with the render targets
    m_framebuffer.Bind();

    renderer.GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // for all drawcalls
    for (const Renderer::DrawcallInfo& drawcallInfo : drawcallCollection)
    {
        assert(drawcallInfo.material.GetBlendEquationColor() == Material::BlendEquation::None);
        assert(drawcallInfo.material.GetBlendEquationAlpha() == Material::BlendEquation::None);
        assert(drawcallInfo.material.GetDepthWrite());

        // Prepare drawcall (similar to forward)
        renderer.PrepareDrawcall(drawcallInfo);

        // Render drawcall
        drawcallInfo.drawcall.Draw();
    }

    // Unbind the framebuffer
    FramebufferObject::Unbind();
}
