#pragma once
#include "./include.hpp"
#include "Application/windows/injector.hpp"
#include "Geometry.h"
#include "Platform.h"
#include "ScintillaTypes.h"

#include <clay/raywrapped.hpp>
#include <memory>

namespace soil {

    namespace sc = Scintilla::Internal;

    class ClaySurface final : public sc::Surface {
    public:
        void clear() noexcept { UnloadTexture(this->render_texture); }

        void Init(sc::WindowID /*window id*/) override{};
        void Init(sc::SurfaceID surface_id, sc::WindowID /*window id*/) override {
            this->render_texture = *static_cast<Texture2D*>(surface_id);
        }

        std::unique_ptr<sc::Surface> AllocatePixMap(int width, int height) override;

        void SetMode(sc::SurfaceMode mode) override { this->surface_mode = mode; }
        void Release() noexcept override { this->clear(); }
        i32  SupportsFeature(Scintilla::Supports feature) noexcept override;
        bool Initialised() override { return this->render_texture.width != 0; }
        i32  LogPixelsY() override;
        int  PixelDivisions() override;
        int  DeviceHeightFont(int points) override;
        void LineDraw(sc::Point start, sc::Point end, sc::Stroke stroke) override;
        void PolyLine(const sc::Point* pts, size_t npts, sc::Stroke stroke) override;
        void Polygon(const sc::Point* pts, size_t npts, sc::FillStroke fill_stroke) override;

        void RectangleDraw(sc::PRectangle rc, sc::FillStroke fill_stroke) override;
        void RectangleFrame(sc::PRectangle rc, sc::Stroke stroke) override;
        void FillRectangle(sc::PRectangle rc, sc::Fill fill) override;
        void FillRectangleAligned(sc::PRectangle rc, sc::Fill fill) override;
        void FillRectangle(sc::PRectangle rc, Surface& surfacePattern) override;
        void RoundedRectangle(sc::PRectangle rc, sc::FillStroke fill_stroke) override;
        void AlphaRectangle(
            sc::PRectangle rc, sc::XYPOSITION cornerSize, sc::FillStroke fill_stroke
        ) override;

        void GradientRectangle(
            sc::PRectangle rc, const std::vector<sc::ColourStop>& stops,
            ClaySurface::GradientOptions options
        ) override;
        void DrawRGBAImage(
            sc::PRectangle rc, int width, int height, const unsigned char* pixelsImage
        ) override;
        void Ellipse(sc::PRectangle rc, sc::FillStroke fill_stroke) override;
        void Stadium(sc::PRectangle rc, sc::FillStroke fill_stroke, Ends ends) override;
        void Copy(sc::PRectangle rc, sc::Point from, Surface& surfaceSource) override;
        std::unique_ptr<sc::IScreenLineLayout> Layout(const sc::IScreenLine* screenLine
        ) override;

        void DrawTextNoClip(
            sc::PRectangle rc, const sc::Font* font_, sc::XYPOSITION ybase,
            std::string_view text, sc::ColourRGBA fore, sc::ColourRGBA back
        ) override;
        void DrawTextClipped(
            sc::PRectangle rc, const sc::Font* font_, sc::XYPOSITION ybase,
            std::string_view text, sc::ColourRGBA fore, sc::ColourRGBA back
        ) override;
        void DrawTextTransparent(
            sc::PRectangle rc, const sc::Font* font_, sc::XYPOSITION ybase,
            std::string_view text, sc::ColourRGBA fore
        ) override;
        void MeasureWidths(
            const sc::Font* font_, std::string_view text, sc::XYPOSITION* positions
        ) override;

        sc::XYPOSITION WidthText(const sc::Font* font_, std::string_view text) override;

        void DrawTextNoClipUTF8(
            sc::PRectangle rc, const sc::Font* font_, sc::XYPOSITION ybase,
            std::string_view text, sc::ColourRGBA fore, sc::ColourRGBA back
        ) override;
        void DrawTextClippedUTF8(
            sc::PRectangle rc, const sc::Font* font_, sc::XYPOSITION ybase,
            std::string_view text, sc::ColourRGBA fore, sc::ColourRGBA back
        ) override;
        void DrawTextTransparentUTF8(
            sc::PRectangle rc, const sc::Font* font_, sc::XYPOSITION ybase,
            std::string_view text, sc::ColourRGBA fore
        ) override;
        void MeasureWidthsUTF8(
            const sc::Font* font_, std::string_view text, sc::XYPOSITION* positions
        ) override;
        sc::XYPOSITION WidthTextUTF8(const sc::Font* font_, std::string_view text) override;
        sc::XYPOSITION Ascent(const sc::Font* font_) override;
        sc::XYPOSITION Descent(const sc::Font* font_) override;
        sc::XYPOSITION InternalLeading(const sc::Font* font_) override;
        sc::XYPOSITION Height(const sc::Font* font_) override;
        sc::XYPOSITION AverageCharWidth(const sc::Font* font_) override;
        void           SetClip(sc::PRectangle rc) override;
        void           PopClip() override;
        void           FlushCachedState() override;
        void           FlushDrawing() override;

        ClaySurface(Vec2i size, sc::SurfaceMode mode);

        ~ClaySurface() override { this->clear(); };

    private:
        void compute_scale_factor();

        static Vector2 s_to_c(sc::Point val) {
            return {.x = static_cast<f32>(val.x), .y = static_cast<f32>(val.y)};
        }

        static Ray_Color s_to_c(sc::ColourRGBA val) {
            return {
                .r = val.GetRed(), .g = val.GetGreen(), .b = val.GetRed(), .a = val.GetAlpha()
            };
        }

        static Ray_Rectangle s_to_c(sc::PRectangle val) {
            auto top_left = Vector2{static_cast<f32>(val.left), static_cast<f32>(val.top)};
            return Ray_Rectangle{
                .x      = top_left.x,
                .y      = top_left.y,
                .width  = static_cast<f32>(val.Width()),
                .height = static_cast<f32>(val.Height())
            };
        }

    private:
        Texture2D       render_texture{0, 0, 0, 0, 0};
        sc::SurfaceMode surface_mode;
        i32             scale_factor{1};
    };

} // namespace soil