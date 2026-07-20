#include <cstdio>
#include <cmath>
#include <vector>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main() {
    constexpr int irLen   = 1024;
    constexpr int fftSize = irLen * 2;
    const double sampleRate = 44100.0;
    const double nyquist = sampleRate / 2.0;

    // Step 1: correction curve
    std::vector<double> corrDB(irLen);
    for (int k = 0; k < irLen; ++k) {
        const double f = k * nyquist / (irLen - 1);
        double m50x = 0.0;
        if (f < 500.0) {
            const double w = f / 120.0;
            m50x += 6.0 / (1.0 + w * w);
        }
        if (f > 600.0) {
            const double fk = f / 10000.0;
            m50x += 4.0 * std::exp(-std::pow((fk - 1.0) / 0.4, 2.0));
        }
        double harman = 0.0;
        if (f < 150.0)
            harman += 5.5 * (1.0 - f / 150.0);
        if (f > 8000.0)
            harman -= 0.5 * (f - 8000.0) / 12000.0;
        double db = harman - m50x;
        if (db > 12.0) db = 12.0;
        if (db < -12.0) db = -12.0;
        corrDB[k] = db;
    }

    // Step 2: symmetric spectrum
    std::vector<double> specR(fftSize);
    for (int k = 0; k < irLen; ++k)
        specR[k] = std::pow(10.0, corrDB[k] / 20.0);
    for (int k = irLen; k < fftSize; ++k)
        specR[k] = specR[fftSize - k];

    // Step 3: IFFT
    std::vector<double> ir(irLen);
    const double twopiN = 2.0 * M_PI / fftSize;
    for (int n = 0; n < irLen; ++n) {
        double sum = 0.0;
        for (int k = 0; k < fftSize; ++k)
            sum += specR[k] * std::cos(twopiN * k * n);
        ir[n] = sum / fftSize;
    }

    // Print as C++ static array
    printf("// ATH-M50X to Harman 2018 OE, 44100 Hz, 1024 taps, minimum-phase\n");
    printf("static const float firData[1024] = {\n");
    for (int i = 0; i < irLen; ++i) {
        if (i % 8 == 0) printf("   ");
        printf("% .10ff,", ir[i]);
        if (i % 8 == 7) printf("\n");
    }
    printf("\n};\n");
    return 0;
}
