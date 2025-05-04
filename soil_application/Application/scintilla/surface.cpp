#include <GLFW/glfw3.h>
#include <cassert>
#include <print>
#include <span>

#include "../../clay/render.hpp"
#include "./surface.hpp"
#include "Platform.h"
#include "ScintillaTypes.h"
#include "raylib.h"
#include <algorithm>
#include <array>
#include <memory>


#include <ranges>

namespace soil {
    using namespace Scintilla;

    constexpr i32 ppi = 72;

    inline const std::array<Supports, 4> Supports = {
        Supports::FractionalStrokeWidth,
        Supports::TranslucentStroke,
        Supports::PixelModification,
        Supports::ThreadSafeMeasureWidths,
    };

    std::unique_ptr<sc::Surface> ClaySurface::AllocatePixMap(int width, int height) {
        return std::make_unique<ClaySurface>(Vec2i{width, height}, this->surface_mode);
    }

    i32 ClaySurface::SupportsFeature(Scintilla::Supports feature) noexcept {
        return static_cast<i32>(std::ranges::contains(Supports, feature));
    }

    i32 ClaySurface::LogPixelsY() {
        constexpr i32 standard_dpi = 96;
        return standard_dpi;
    }

    ClaySurface::ClaySurface(Vec2i size, sc::SurfaceMode mode) {
        if (this->render_texture.height != 0) {
            UnloadTexture(this->render_texture);
        }

        this->surface_mode = mode;

        const auto texture   = GenImageColor(size.x, size.y, {0, 0, 0, 255});
        this->render_texture = LoadTextureFromImage(texture);
        UnloadImage(texture);

        this->compute_scale_factor();
    }

    i32 ClaySurface::PixelDivisions() { return this->scale_factor; }

    int ClaySurface::DeviceHeightFont(int points) {
        return (points * this->LogPixelsY()) / ppi;
    }

    void ClaySurface::LineDraw(sc::Point start, sc::Point end, sc::Stroke stroke) {
        DrawLineEx(
            ClaySurface::s_to_c(start), ClaySurface::s_to_c(end), stroke.WidthF(),
            ClaySurface::s_to_c(stroke.colour)
        );
    }

    void ClaySurface::PolyLine(const sc::Point* pts, size_t npts, sc::Stroke stroke) {
        const auto points = std::span{pts, npts};

        for (const auto& val : points | std::views::slide(2)) {
            const auto& start = val[0];
            const auto& end   = val[1];

            this->LineDraw(start, end, stroke);
        }
    }

    void ClaySurface::Polygon(const sc::Point* pts, size_t npts, sc::FillStroke fill_stroke) {
        std::println("Unimplemented call to Polygon drawer!");
        (void)pts;
        (void)npts;
        (void)fill_stroke;
    }

    void ClaySurface::RectangleDraw(sc::PRectangle rc, sc::FillStroke fill_stroke) {
        DrawRectangleRec(ClaySurface::s_to_c(rc), ClaySurface::s_to_c(fill_stroke.fill.colour));

        // TODO(Akashic): Make this correct
        if (fill_stroke.fill.colour != fill_stroke.stroke.colour) {
            std::println("Rectangle draw needs more impl");
        }
    }

    void ClaySurface::RectangleFrame(sc::PRectangle rc, sc::Stroke stroke) {
        DrawRectangleLinesEx(
            ClaySurface::s_to_c(rc), stroke.WidthF(), ClaySurface::s_to_c(stroke.colour)
        );
    }

    void ClaySurface::FillRectangle(sc::PRectangle rc, sc::Fill fill) {
        DrawRectangleRec(ClaySurface::s_to_c(rc), ClaySurface::s_to_c(fill.colour));
    }

    void ClaySurface::FillRectangleAligned(sc::PRectangle rc, sc::Fill fill) {
        this->FillRectangle(sc::PixelAlign(rc, this->scale_factor), fill);
    }

    void ClaySurface::FillRectangle(sc::PRectangle rc, Surface& surfacePattern) {
        std::println("Unimplemented rectangle fill");
        (void)rc;
        (void)surfacePattern;
    }

    void ClaySurface::RoundedRectangle(sc::PRectangle rc, sc::FillStroke fill_stroke) {

        // Stolen from the D2D impl
        const f32 min_dimension = static_cast<f32>(std::min(rc.Width(), rc.Height())) / 2.0F;
        const f32 radius        = std::min(4.0F, min_dimension);

        this->AlphaRectangle(rc, radius, fill_stroke);
    }

    void ClaySurface::AlphaRectangle(
        sc::PRectangle rc, sc::XYPOSITION cornerSize, sc::FillStroke fill_stroke
    ) {
        const f32 radius = static_cast<f32>(cornerSize);

        constexpr i32 segments = 32; // Have no clue if this is a good number

        if (fill_stroke.fill.colour == fill_stroke.stroke.colour) {
            DrawRectangleRounded(
                ClaySurface::s_to_c(rc), radius, segments,
                ClaySurface::s_to_c(fill_stroke.fill.colour)
            );
        } else {
            DrawRectangleRounded(
                ClaySurface::s_to_c(rc), radius, segments,
                ClaySurface::s_to_c(fill_stroke.fill.colour)
            );
            DrawRectangleLinesEx(
                ClaySurface::s_to_c(rc), fill_stroke.stroke.WidthF(),
                ClaySurface::s_to_c(fill_stroke.stroke.colour)
            );
        }
    }

    void ClaySurface::compute_scale_factor() {
        i32         count{};
        auto* const monitors = glfwGetMonitors(&count);

        const auto current_monitor = GetCurrentMonitor();

        auto* const monitor = monitors[current_monitor];

        f32 x_dpi{};
        f32 y_dpi{};
        glfwGetMonitorContentScale(monitor, &x_dpi, &y_dpi);

        this->scale_factor = static_cast<i32>(x_dpi);
        (void)y_dpi;
    }

    void ClaySurface::GradientRectangle(
        sc::PRectangle rc, const std::vector<sc::ColourStop>& stops,
        ClaySurface::GradientOptions options
    ) {
        assert(!"Unimplemented");
        (void)rc;
        (void)stops;
        (void)options;
    }

    void ClaySurface::DrawRGBAImage(
        sc::PRectangle rc, int width, int height, const unsigned char* pixelsImage
    ) {
        assert(!"Unimplemented");
        (void)rc;
        (void)height;
        (void)width;
        (void)pixelsImage;
    }

    void ClaySurface::Ellipse(sc::PRectangle rc, sc::FillStroke fill_stroke) {
        assert(!"Unimplemented");
        (void)rc;
        (void)fill_stroke;
        // This is easy enough since raylib handles it, just dont implement stuff which isnt
        // needed
    }

    void ClaySurface::Stadium(sc::PRectangle rc, sc::FillStroke fill_stroke, Ends ends) {
        std::println("inproper call to stadium!");
        this->FillRectangle(rc, fill_stroke.fill);
        (void)ends;
    }

    void ClaySurface::Copy(sc::PRectangle rc, sc::Point from, Surface& surfaceSource) {
        std::println("Unhandled Copy");
        (void)rc;
        (void)from;
        (void)surfaceSource;
    }

    std::unique_ptr<sc::IScreenLineLayout> ClaySurface::Layout(const sc::IScreenLine* screenLine
    ) {
        assert(!"Unhandled layout call!");
        (void)screenLine;
        return nullptr;
    }

    void ClaySurface::DrawTextNoClip(
        sc::PRectangle rc, const sc::Font* font_, sc::XYPOSITION ybase, std::string_view text,
        sc::ColourRGBA fore, sc::ColourRGBA back
    ) {
        raylib_renderer::CustomDrawTextEx()
    }
    void ClaySurface::DrawTextClipped(
        sc::PRectangle rc, const sc::Font* font_, sc::XYPOSITION ybase, std::string_view text,
        sc::ColourRGBA fore, sc::ColourRGBA back
    ) {}
    void ClaySurface::DrawTextTransparent(
        sc::PRectangle rc, const sc::Font* font_, sc::XYPOSITION ybase, std::string_view text,
        sc::ColourRGBA fore
    ) {}
    void ClaySurface::MeasureWidths(
        const sc::Font* font_, std::string_view text, sc::XYPOSITION* positions
    ) {}
} // namespace soil