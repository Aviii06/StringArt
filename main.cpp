#include "Vivid.h"
#include <Magick++.h>
#define PI 3.14159265f
#define TOTAL_POINTS 100
const int fidelity = 100;
const int scale = 4;

String fileNameArg;
std::vector<float> g_Pixels;
std::vector<std::pair<int, int>> g_Lines;

void ImageToPixels(String fileName)
{
    Magick::Image image(fileName);
    Magick::Blob blob;
    image.resize(Magick::Geometry(fidelity, fidelity));

    image.write(&blob, "RGB");
    for (int i = 0; i < fidelity; i += 1)
    {
        for (int j = 0; j < fidelity; j += 1)
        {
            Magick::ColorRGB px = image.pixelColor(i, j);
            float col = (px.red() + px.green() + px.blue()) / 3;
            g_Pixels.push_back(col * 255 + 1.0f);
        }
    }

//    std::string str((char*)blob.data(), blob.length());
//    for (int i = 0; i < str.length(); i += 3)
//    {
//        g_Pixels.push_back(((float)str[i] + (float)str[i + 1] + (float)str[i + 2]) / 3.0f);
//    }
}

void LinesToPixel(std::vector<float>& pixels, int totalPoints)
{
    for (auto&& px : pixels)
    {
        px = 0.0f;
    }

    for (auto line : g_Lines)
    {
        float angle1 = line.first * 2 * PI / totalPoints;
        float angle2 = line.second * 2 * PI / totalPoints;
        Vec2 point1 = {cos(angle1), sin(angle1)};
        Vec2 point2 = {cos(angle2), sin(angle2)};

        float m = (point2.y - point1.y) / (point2.x - point1.x);
        float c = point1.y - m * point1.x;
        for (int y = 0; y < fidelity; y++)
        {
            for (int x = 0; x < fidelity; x++)
            {
                float pixelValue = (pixels)[y * fidelity + x];

                float dist = abs(m * x + c - y) / sqrt(m * m + 1);
                float addValue = 255 * exp(-dist * dist / 2.0f) + 1.0f;

                pixelValue = std::min(256.0f, pixelValue + addValue);

                (pixels)[y * fidelity + x] = pixelValue;
            }
        }
    }
}

void ProgressBar(int curr, int total)
{
    float progress = (float)curr / (float)total;
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i)
    {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0f) << " %\r";
    std::cout.flush();
}

class ExampleInterface : public RenderingInterface
{
public:
    void Setup() override
    {
        OPENGL_2D_CONFS

        // Figure out the lines
        std::vector<float> temp_pixels;
        temp_pixels.reserve(fidelity * fidelity);
        for (int i = 0; i < fidelity * fidelity; i++)
        {
            temp_pixels.push_back(0.0f);
        }
        g_Lines.reserve(TOTAL_POINTS - 1);

        int j = 0;

        std::vector<int> possiblePoints;
        for (int i = 0; i < TOTAL_POINTS; i++) {
            possiblePoints.push_back(i);
        }

        int count = 0;

        while(g_Lines.size() < TOTAL_POINTS - 1) {
            float minDistance = INFINITY;
            std::pair<int, int> bestLine;

            for (int i : possiblePoints) {
                if (i == j) {
                    continue;
                }

                g_Lines.push_back({j, i});
                LinesToPixel(temp_pixels, TOTAL_POINTS);

                // Find distance between pixels
                float distance = 0.0f;
                for (int y = 0; y < fidelity; y++) {
                    for (int x = 0; x < fidelity; x++) {
                        distance += ((temp_pixels)[y * fidelity + x] - g_Pixels[y * fidelity + x]) *
                                    ((temp_pixels)[y * fidelity + x] - g_Pixels[y * fidelity + x]);
                    }
                }

                // If distance is lowest then keep the line
                if (distance < minDistance) {
                    minDistance = distance;
                    bestLine = {j, i};
                }
                g_Lines.pop_back();
            }
            j = bestLine.second;
            g_Lines.push_back(bestLine);
            possiblePoints.erase(std::remove(possiblePoints.begin(), possiblePoints.end(), bestLine.first), possiblePoints.end());

            count++;
            ProgressBar(count, TOTAL_POINTS - 1);
        }

        Vivid::Renderer2D::Init();
    }

    void Draw() override
    {
        Vivid::Renderer2D::BeginScene();

        // Draw using API
        Vivid::Renderer2D::DrawQuad(0, 0, 1920*2, 1080*2, Vec3(0.0f, 0.0f, 0.0f));
        Vivid::Renderer2D::DrawCircle(Vec2(0, 0), fidelity * scale, Vec3(1.0f, 1.0f, 1.0f));

        // draw the lines
        for (auto line : g_Lines)
        {
            float angle1 = line.first * 2 * PI / TOTAL_POINTS;
            float angle2 = line.second * 2 * PI / TOTAL_POINTS;
            Vec2 point1 = {cos(angle1), sin(angle1)};
            Vec2 point2 = {cos(angle2), sin(angle2)};

            Vivid::Renderer2D::DrawLine(Vec2(point1.x * fidelity * scale, point1.y * fidelity * scale),
                                        Vec2(point2.x * fidelity * scale, point2.y * fidelity * scale),
                                        1.0f,
                                        Vec3(0.0f, 0.0f, 0.0f));
        }


        Vivid::Renderer2D::EndScene();

        Vivid::Renderer2D::BeginScene();

        // draw the pixels
//        for (int y = 0; y < fidelity; y++)
//        {
//            for (int x = 0; x < fidelity; x++)
//            {
//                float pixelValue = (g_Pixels)[x * fidelity + y];
//                float color = pixelValue / 256.0f;
//                Vivid::Renderer2D::DrawQuad((-x + fidelity/2) * scale, (-y + fidelity/2) * scale, scale, scale, Vec3(color, color, color));
//            }
//        }

        Vivid::Renderer2D::EndScene();
    }

    void ImGuiRender() override
    {
        ImGui::Begin("Stats");

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();

        ImGui::Begin("Image Importer");
        ImGui::End();
    }
};

Application* Vivid::CreateApplication()
{
    Application* app = Application::GetInstance(1920, 1080, "Rendering2D");

    OrthoCamera* orthoCamera = new OrthoCamera(0, 1920, 0, 1080);
    app->SetCamera(orthoCamera);
    app->SetRenderingInterface(new ExampleInterface);
    return app;
}

int main(int argc, char** argv)
{
//    if (argv[1] == nullptr)
//    {
//        std::cout << "Please provide a file name!" << std::endl;
//        return 0;
//    }
    fileNameArg = "../assets/monalisa.png";

    ImageToPixels(fileNameArg);

    return Vivid::main(0, nullptr);
}