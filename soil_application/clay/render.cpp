#include "render.hpp"
#include "../types.hpp"
#include "./clay_binding.hpp"
#include "./components.hpp"
#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace raylib_renderer {
#include "raylib.h"
#include "raymath.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

    void CustomDrawTextEx(
        Font font, std::string_view text, Vector2 position, float fontSize, float spacing,
        Color tint
    ) {
        if (font.texture.id == 0)
            font = GetFontDefault(); // Security check in case of not valid font

        int size =
            text.size(); // Total size in bytes of the text, scanned by codepoints in loop

        float textOffsetY = 0;    // Offset between lines (on linebreak '\n')
        float textOffsetX = 0.0f; // Offset X to next character to draw

        float scaleFactor = fontSize / font.baseSize; // Character quad scaling factor

        for (int i = 0; i < size;) {
            // Get next codepoint from byte string and glyph index in font
            int codepointByteCount = 0;
            int codepoint          = GetCodepointNext(&text[i], &codepointByteCount);
            int index              = GetGlyphIndex(font, codepoint);

            if (codepoint == '\n') {
                static int textLineSpacing = 2; // TODO change
                // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
                textOffsetY += (fontSize + textLineSpacing);
                textOffsetX  = 0.0f;
            } else {
                if ((codepoint != ' ') && (codepoint != '\t')) {
                    DrawTextCodepoint(
                        font, codepoint,
                        (Vector2){position.x + textOffsetX, position.y + textOffsetY}, fontSize,
                        tint
                    );
                }

                if (font.glyphs[index].advanceX == 0)
                    textOffsetX += ((float)font.recs[index].width * scaleFactor + spacing);
                else
                    textOffsetX += ((float)font.glyphs[index].advanceX * scaleFactor + spacing);
            }

            i += codepointByteCount; // Move text bytes counter to next codepoint
        }
    }

#define CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(rectangle)                                          \
    (Rectangle) {                                                                              \
        .x = rectangle.x, .y = rectangle.y, .width = rectangle.width,                          \
        .height = rectangle.height                                                             \
    }
#define CLAY_COLOR_TO_RAYLIB_COLOR(color)                                                      \
    (Color) {                                                                                  \
        .r = (unsigned char)roundf(color.r), .g = (unsigned char)roundf(color.g),              \
        .b = (unsigned char)roundf(color.b), .a = (unsigned char)roundf(color.a)               \
    }

    Camera Raylib_camera;

    // Get a ray trace from the screen position (i.e mouse) within a specific section of the
    // screen
    Ray GetScreenToWorldPointWithZDistance(
        Vector2 position, Camera camera, int screenWidth, int screenHeight, float zDistance
    ) {
        Ray ray = {0};

        // Calculate normalized device coordinates
        // NOTE: y value is negative
        float x = (2.0f * position.x) / (float)screenWidth - 1.0f;
        float y = 1.0f - (2.0f * position.y) / (float)screenHeight;
        float z = 1.0f;

        // Store values in a vector
        Vector3 deviceCoords = {x, y, z};

        // Calculate view matrix from camera look at
        Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

        Matrix matProj = MatrixIdentity();

        if (camera.projection == CAMERA_PERSPECTIVE) {
            // Calculate projection matrix from perspective
            matProj = MatrixPerspective(
                camera.fovy * DEG2RAD, ((double)screenWidth / (double)screenHeight), 0.01f,
                zDistance
            );
        } else if (camera.projection == CAMERA_ORTHOGRAPHIC) {
            double aspect = (double)screenWidth / (double)screenHeight;
            double top    = camera.fovy / 2.0;
            double right  = top * aspect;

            // Calculate projection matrix from orthographic
            matProj = MatrixOrtho(-right, right, -top, top, 0.01, 1000.0);
        }

        // Unproject far/near points
        Vector3 nearPoint =
            Vector3Unproject((Vector3){deviceCoords.x, deviceCoords.y, 0.0f}, matProj, matView);
        Vector3 farPoint =
            Vector3Unproject((Vector3){deviceCoords.x, deviceCoords.y, 1.0f}, matProj, matView);

        // Calculate normalized direction vector
        Vector3 direction = Vector3Normalize(Vector3Subtract(farPoint, nearPoint));

        ray.position = farPoint;

        // Apply calculated vectors to ray
        ray.direction = direction;

        return ray;
    }

    Clay_Dimensions Raylib_MeasureText(
        Clay_StringSlice text, Clay_TextElementConfig* config, void* userData
    ) noexcept {
        // Measure string size for Font
        Clay_Dimensions textSize = {0};

        float maxTextWidth  = 0.0f;
        float lineTextWidth = 0;

        float textHeight = config->fontSize;
        Font* fonts      = (Font*)userData;
        Font  fontToUse  = fonts[config->fontId];
        // Font failed to load, likely the fonts are in the wrong place relative to the
        // execution dir. RayLib ships with a default font, so we can continue with that built
        // in one.
        if (!fontToUse.glyphs) {
            fontToUse = GetFontDefault();
        }

        float scaleFactor = config->fontSize / (float)fontToUse.baseSize;

        for (int i = 0; i < text.length; ++i) {
            if (text.chars[i] == '\n') {
                maxTextWidth  = fmax(maxTextWidth, lineTextWidth);
                lineTextWidth = 0;
                continue;
            }
            int index = text.chars[i] - 32;
            if (fontToUse.glyphs[index].advanceX != 0)
                lineTextWidth += fontToUse.glyphs[index].advanceX;
            else
                lineTextWidth +=
                    (fontToUse.recs[index].width + fontToUse.glyphs[index].offsetX);
        }

        maxTextWidth = fmax(maxTextWidth, lineTextWidth);

        textSize.width  = maxTextWidth * scaleFactor;
        textSize.height = textHeight;

        return textSize;
    }

    void Clay_Raylib_Initialize(
        int width, int height, const char* title, unsigned int flags
    ) noexcept {
        SetConfigFlags(flags);
        InitWindow(width, height, title);
        //    EnableEventWaiting();
    }

    // A MALLOC'd buffer, that we keep modifying inorder to save from so many Malloc and Free
    // Calls. Call Clay_Raylib_Close() to free

    void Clay_Raylib_Render(Clay_RenderCommandArray renderCommands, Font* fonts) noexcept {
        for (int j = 0; j < renderCommands.length; j++) {
            Clay_RenderCommand* renderCommand = Clay_RenderCommandArray_Get(&renderCommands, j);
            Clay_BoundingBox    boundingBox   = renderCommand->boundingBox;
            switch (renderCommand->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                Clay_TextRenderData* textData  = &renderCommand->renderData.text;
                Font                 fontToUse = fonts[textData->fontId];

                // Modified version of raylibs drawing function, which allows non nullterm
                // strings
                CustomDrawTextEx(
                    fontToUse,
                    std::string_view{
                        textData->stringContents.chars, (usize)textData->stringContents.length
                    },
                    (Vector2){boundingBox.x, boundingBox.y}, (float)textData->fontSize,
                    (float)textData->letterSpacing,
                    CLAY_COLOR_TO_RAYLIB_COLOR(textData->textColor)
                );
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                Texture2D imageTexture = *(Texture2D*)renderCommand->renderData.image.imageData;
                Clay_Color tintColor   = renderCommand->renderData.image.backgroundColor;
                if (tintColor.r == 0 && tintColor.g == 0 && tintColor.b == 0 &&
                    tintColor.a == 0) {
                    tintColor = (Clay_Color){255, 255, 255, 255};
                }
                DrawTextureEx(
                    imageTexture, (Vector2){boundingBox.x, boundingBox.y}, 0,
                    boundingBox.width / (float)imageTexture.width,
                    CLAY_COLOR_TO_RAYLIB_COLOR(tintColor)
                );
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                BeginScissorMode(
                    (int)roundf(boundingBox.x), (int)roundf(boundingBox.y),
                    (int)roundf(boundingBox.width), (int)roundf(boundingBox.height)
                );
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                EndScissorMode();
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                Clay_RectangleRenderData* config = &renderCommand->renderData.rectangle;
                if (config->cornerRadius.topLeft > 0) {
                    float radius =
                        (config->cornerRadius.topLeft * 2) /
                        (float)((boundingBox.width > boundingBox.height) ? boundingBox.height
                                                                         : boundingBox.width);
                    DrawRectangleRounded(
                        (Rectangle){boundingBox.x, boundingBox.y, boundingBox.width,
                                    boundingBox.height},
                        radius, 8, CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor)
                    );
                } else {
                    DrawRectangle(
                        boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height,
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor)
                    );
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                Clay_BorderRenderData* config = &renderCommand->renderData.border;
                // Left border
                if (config->width.left > 0) {
                    DrawRectangle(
                        (int)roundf(boundingBox.x),
                        (int)roundf(boundingBox.y + config->cornerRadius.topLeft),
                        (int)config->width.left,
                        (int)roundf(
                            boundingBox.height - config->cornerRadius.topLeft -
                            config->cornerRadius.bottomLeft
                        ),
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                // Right border
                if (config->width.right > 0) {
                    DrawRectangle(
                        (int)roundf(boundingBox.x + boundingBox.width - config->width.right),
                        (int)roundf(boundingBox.y + config->cornerRadius.topRight),
                        (int)config->width.right,
                        (int)roundf(
                            boundingBox.height - config->cornerRadius.topRight -
                            config->cornerRadius.bottomRight
                        ),
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                // Top border
                if (config->width.top > 0) {
                    DrawRectangle(
                        (int)roundf(boundingBox.x + config->cornerRadius.topLeft),
                        (int)roundf(boundingBox.y),
                        (int)roundf(
                            boundingBox.width - config->cornerRadius.topLeft -
                            config->cornerRadius.topRight
                        ),
                        (int)config->width.top, CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                // Bottom border
                if (config->width.bottom > 0) {
                    DrawRectangle(
                        (int)roundf(boundingBox.x + config->cornerRadius.bottomLeft),
                        (int)roundf(boundingBox.y + boundingBox.height - config->width.bottom),
                        (int)roundf(
                            boundingBox.width - config->cornerRadius.bottomLeft -
                            config->cornerRadius.bottomRight
                        ),
                        (int)config->width.bottom, CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                if (config->cornerRadius.topLeft > 0) {
                    DrawRing(
                        (Vector2){roundf(boundingBox.x + config->cornerRadius.topLeft),
                                  roundf(boundingBox.y + config->cornerRadius.topLeft)},
                        roundf(config->cornerRadius.topLeft - config->width.top),
                        config->cornerRadius.topLeft, 180, 270, 10,
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                if (config->cornerRadius.topRight > 0) {
                    DrawRing(
                        (Vector2
                        ){roundf(
                              boundingBox.x + boundingBox.width - config->cornerRadius.topRight
                          ),
                          roundf(boundingBox.y + config->cornerRadius.topRight)},
                        roundf(config->cornerRadius.topRight - config->width.top),
                        config->cornerRadius.topRight, 270, 360, 10,
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                if (config->cornerRadius.bottomLeft > 0) {
                    DrawRing(
                        (Vector2){roundf(boundingBox.x + config->cornerRadius.bottomLeft),
                                  roundf(
                                      boundingBox.y + boundingBox.height -
                                      config->cornerRadius.bottomLeft
                                  )},
                        roundf(config->cornerRadius.bottomLeft - config->width.top),
                        config->cornerRadius.bottomLeft, 90, 180, 10,
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                if (config->cornerRadius.bottomRight > 0) {
                    DrawRing(
                        (Vector2){roundf(
                                      boundingBox.x + boundingBox.width -
                                      config->cornerRadius.bottomRight
                                  ),
                                  roundf(
                                      boundingBox.y + boundingBox.height -
                                      config->cornerRadius.bottomRight
                                  )},
                        roundf(config->cornerRadius.bottomRight - config->width.bottom),
                        config->cornerRadius.bottomRight, 0.1, 90, 10,
                        CLAY_COLOR_TO_RAYLIB_COLOR(config->color)
                    );
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
                Clay_CustomRenderData* config        = &renderCommand->renderData.custom;
                CustomLayoutElement*   customElement = (CustomLayoutElement*)config->customData;
                if (!customElement) continue;
                switch (customElement->type) {
                case CUSTOM_LAYOUT_ELEMENT_TYPE_3D_MODEL: {
                    Clay_BoundingBox rootBox    = renderCommands.internalArray[0].boundingBox;
                    float            scaleValue = CLAY__MIN(
                        CLAY__MIN(1, 768 / rootBox.height) * CLAY__MAX(1, rootBox.width / 1024),
                        1.5f
                    );
                    Ray positionRay = GetScreenToWorldPointWithZDistance(
                        (Vector2
                        ){renderCommand->boundingBox.x + renderCommand->boundingBox.width / 2,
                          renderCommand->boundingBox.y +
                              (renderCommand->boundingBox.height / 2) + 20},
                        Raylib_camera, (int)roundf(rootBox.width), (int)roundf(rootBox.height),
                        140
                    );
                    BeginMode3D(Raylib_camera);
                    DrawModel(
                        customElement->customData.model.model, positionRay.position,
                        customElement->customData.model.scale * scaleValue, WHITE
                    ); // Draw 3d model with texture
                    EndMode3D();
                    break;
                }
                default:
                    break;
                }
                break;
            }
            default: {
                std::terminate();
            }
            }
        }
    }
} // namespace raylib_renderer

namespace clay_extension {
    void render_command_list(Clay_RenderCommandArray commands, std::span<Font> fonts) noexcept {
        for (usize index = 0; index < commands.length; index++) {
            Clay_RenderCommand& command = commands.internalArray[index];

            if (command.commandType !=
                Clay_RenderCommandType::CLAY_RENDER_COMMAND_TYPE_CUSTOM) {
                raylib_render_command_passthrough(
                    Clay_RenderCommandArray{1, 1, &command}, fonts.data()
                );
                continue;
            }

            const auto* type = (ComponentType*)command.userData;

            if (*type == ComponentType::RAYLIB_3D_MODEL) {
                raylib_render_command_passthrough(
                    Clay_RenderCommandArray{1, 1, &command}, fonts.data()
                );
                continue;
            }

            if (*type == ComponentType::CUSTOM_VIRTUAL) {
                const auto* data = (RenderComponentStore*)command.userData;

                data->component->on_render();
                continue;
            }

            std::unreachable();
            // This throw is fine since we want to kill the application anyway.
            // So its ok to just call std::terminate
            // NOLINTNEXTLINE
            throw std::logic_error("Unhandled Draw Command");
        }
    }
} // namespace clay_extension