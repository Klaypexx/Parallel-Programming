#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <windows.h>
#include <chrono>

const int KERNEL_SIZE = 10;

struct ThreadData {
    const cv::Mat* input;
    cv::Mat* output;
    int kernelSize; 
    int startRow;
    int endRow;
    int numCores;
};

DWORD WINAPI ThreadProc(LPVOID lpParam) {
    ThreadData* data = static_cast<ThreadData*>(lpParam);

    // Set the affinity mask for this thread
    DWORD_PTR mask = (static_cast<DWORD_PTR>(1) << (data->startRow % data->numCores)); // Use modulo to cycle through cores
    SetThreadAffinityMask(GetCurrentThread(), mask);

    int halfKernel = data->kernelSize / 2;

    for (int y = data->startRow; y < data->endRow; ++y) {
        for (int x = 0; x < data->input->cols; ++x) {
            int rSum = 0, gSum = 0, bSum = 0;
            int count = 0;

            for (int ky = -halfKernel; ky <= halfKernel; ++ky) {
                for (int kx = -halfKernel; kx <= halfKernel; ++kx) {
                    int nx = x + kx;
                    int ny = y + ky;

                    if (nx >= 0 && nx < data->input->cols && ny >= 0 && ny < data->input->rows) {
                        cv::Vec3b pixel = data->input->at<cv::Vec3b>(ny, nx);
                        rSum += pixel[2];
                        gSum += pixel[1];
                        bSum += pixel[0];
                        count++;
                    }
                }
            }

            cv::Vec3b& newPixel = data->output->at<cv::Vec3b>(y, x);
            newPixel[2] = rSum / count;
            newPixel[1] = gSum / count;
            newPixel[0] = bSum / count;
        }
    }

    return 0;
}

void processImage(const std::string& filename, int numThreads, int numCores) {

    // Чтение изображения
    cv::Mat img = cv::imread(filename);
    if (img.empty()) {
        std::cerr << "Error: Could not open or find the image!" << std::endl;
        return;
    }

    cv::Mat blur(img.size(), img.type());

    HANDLE* handles = new HANDLE[numThreads];
    ThreadData* threadDataArray = new ThreadData[numThreads];

    int rowsPerThread = img.rows / numThreads;

    // Создание потоков
    for (int i = 0; i < numThreads; ++i) {
        threadDataArray[i] = { &img, &blur, KERNEL_SIZE,
                                i * rowsPerThread,
                                (i == numThreads - 1) ? img.rows : (i + 1) * rowsPerThread,
                                numCores }; 

        handles[i] = CreateThread(NULL, 0, &ThreadProc, &threadDataArray[i], 0, NULL);
        if (handles[i] == NULL) {
            std::cerr << "Flow error " << i + 1 << std::endl;
            return;
        }
    }

    // Ожидание завершения всех потоков
    WaitForMultipleObjects(numThreads, handles, TRUE, INFINITE);

    // Закрытие дескрипторов потоков
    for (int i = 0; i < numThreads; ++i) {
        CloseHandle(handles[i]);
    }

    delete[] handles;
    delete[] threadDataArray;

    // Запись результата в файл
    cv::imwrite("output.bmp", blur);
}

int main(int argc, char* argv[]) {
    if (argc != 4) { 
        std::cerr << "Usage: " << argv[0] << " <image_path> <num_threads> <num_cores>" << std::endl;
        return -1;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    const std::string filename = argv[1];
    int numThreads = std::atoi(argv[2]);
    int numCores = std::atoi(argv[3]);

    processImage(filename, numThreads, numCores);

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = endTime - startTime;

    std::cout << "Execution time: " << duration.count() << " ms" << std::endl;

    return 0;
}