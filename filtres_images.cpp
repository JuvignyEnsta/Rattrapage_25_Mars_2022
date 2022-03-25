#include <complex>
#include <ratio>
#include <type_traits>
#include <iostream>
#include <vector>
#include <queue>
#include <cstdint>
#include <algorithm>
#include <chrono>
#include <cassert>
#include "lodepng/lodepng.h"

std::vector<double> normaliseEtEtendre( std::uint32_t width, std::uint32_t height, std::uint8_t const* pixels )
{
    std::uint32_t w = width  + 2;
    std::uint32_t h = height + 2;
    std::vector<double> image_etendue(3*w*h,0.);

    for ( int c = 0; c < 3; ++c )
    {
        image_etendue[c] = 0.;
        image_etendue[3*(w-1)+c] = 0.;
        image_etendue[3*w*(h-1) + c] = 0.; 
        image_etendue[3*(w*h-1) + c] = 0.;

        for ( std::uint32_t i = 0; i < width; ++i )
        {
            image_etendue[3*(i+1)+c] = pixels[3*i+c];
            image_etendue[3*(w*(h-1)+i+1)+c] = pixels[3*(width*(height-1)+i)+c];
        }
        for ( std::uint32_t j = 0; j < height; ++j )
        {
            image_etendue[3*(j+1)*w+c] = pixels[3*j*width+c];
            image_etendue[3*((j+2)*w-1)+c] = pixels[3*((j+1)*width-1)+c];
        }
        for ( std::uint32_t i = 0; i < width; ++i )
        {
            for ( std::uint32_t j = 0; j < height; ++j )
            {
                image_etendue[3*((i+1)+(j+1)*w)+c] = pixels[3*(i+j*width)+c];
            }
        }
    }
    return image_etendue;
}

std::vector<std::uint8_t> transformeImage8Bits( std::uint32_t width, std::uint32_t height, std::vector<double> const& ext_image )
{
    std::vector<std::uint8_t> image(3*width*height);
    std::uint32_t w = width + 2;

    for ( int c = 0; c < 3; ++c )
    {
        for ( std::uint32_t i = 0; i < width; ++i )
        {
            for ( std::uint32_t j = 0; j < height; ++j )
            {
                image[3*(i + j*width)+c] = std::uint8_t(ext_image[3*((i+1) + (j+1)*w)+c]);
            }
        }
    }
    return image;
}

std::vector<double> negative( std::uint32_t width, std::uint32_t height, std::vector<double> const& pixels )
{
    std::uint32_t w = width  + 2;
    std::uint32_t h = height + 2;
    std::vector<double> image_negative(3*w*h);
    for ( int c = 0; c < 3; ++c )
    {
        for ( std::uint32_t i = 0; i < w; ++i )
        {
            for ( std::uint32_t j = 0; j < h; ++j )
            {
                image_negative[3*(i+j*w)+c] = 255.-pixels[3*(i+j*w)+c];
            }
        }
    }
    return image_negative;
}

std::vector<double> bord( std::uint32_t width, std::uint32_t height, std::vector<double> const& pixels )
{
    std::uint32_t w = width  + 2;
    std::uint32_t h = height + 2;
    assert(pixels.size() == 3*w*h);
    std::vector<double> image_bord(3*w*h);
    for ( int c = 0; c < 3; ++c )
    {
        // Copie pour les bords (condition limite) :
        for ( std::uint32_t i = 0; i < w; ++i )
        {
            image_bord[3*i+c] = pixels[3*i+c];
            image_bord[3*(i + (h-1)*w)+c] = pixels[3*(i + (h-1)*w)+c];
        }
        for ( std::uint32_t j = 1; j < h; ++j )
        {
            image_bord[3*(j*w)+c] = pixels[3*j*w+c];
            image_bord[3*((j+1)*w-1)+c] = pixels[3*((j+1)*w-1)+c];
        }
        // Puis calcul du bord de l'image 
        for ( std::uint32_t i = 1; i <= width; ++i )
        {
            for ( std::uint32_t j = 1; j <= height; ++j )
            {
                std::uint32_t ind = 3*(i+j*w)+c;
                image_bord[ind] = 4*pixels[ind] - pixels[ind-3] - pixels[ind+3] - pixels[ind - 3*w] - pixels[ind + 3*w];
                if (image_bord[ind] < 0)
                    image_bord[ind] = 0.;
                if (image_bord[ind] > 255.)   
                    image_bord[ind] = 255.;
            }
        }
    }
    return image_bord;
}

std::vector<double> flou_gaussien( std::uint32_t width, std::uint32_t height, std::vector<double> const& pixels )
{
    std::uint32_t w = width  + 2;
    std::uint32_t h = height + 2;
    assert(pixels.size() == 3*w*h);
    std::vector<double> image_gauss(3*w*h);
    for ( int c = 0; c < 3; ++c )
    {
        // Copie pour les bords (condition limite) :
        for ( std::uint32_t i = 0; i < w; ++i )
        {
            image_gauss[3*i+c] = pixels[3*i+c];
            image_gauss[3*(i + (h-1)*w)+c] = pixels[3*(i + (h-1)*w)+c];
        }
        for ( std::uint32_t j = 1; j < h; ++j )
        {
            image_gauss[3*(j*w)+c] = pixels[3*j*w+c];
            image_gauss[3*((j+1)*w-1)+c] = pixels[3*((j+1)*w-1)+c];
        }
        // Puis calcul du bord de l'image 
        int dx = 3, dy = 3*w;
        for ( std::uint32_t i = 1; i <= width; ++i )
        {
            for ( std::uint32_t j = 1; j <= height; ++j )
            {
                std::uint32_t ind = 3*(i+j*w)+c;
                image_gauss[ind] =
                    0.0947416*pixels[ind-dx-dy] + 0.118318*pixels[ind-dy] + 0.0947416*pixels[ind+dx-dy]+
                    0.118318 *pixels[ind-dx   ] + 0.147761*pixels[ind   ] + 0.118318 *pixels[ind+dx   ]+
                    0.0947416*pixels[ind-dx-dy] + 0.118318*pixels[ind-dy] + 0.0947416*pixels[ind+dx-dy];
                if (image_gauss[ind] < 0.)
                    image_gauss[ind] = 0.;
                if (image_gauss[ind] > 255.)   
                    image_gauss[ind] = 255.;
            }
        }
    }
    return image_gauss;

}

std::vector<double> bord2( std::uint32_t width, std::uint32_t height, std::vector<double> const& pixels )
{
    std::uint32_t w = width  + 2;
    std::uint32_t h = height + 2;
    assert(pixels.size() == 3*w*h);
    std::vector<double> image_bord(3*w*h);
    std::uint32_t i,j;
    for ( int c = 0; c < 3; ++c )
    {
        // Copie pour les bords (condition limite) :
        for ( i = 0; i < w; ++i )
        {
            image_bord[3*i+c] = pixels[3*i+c];
            image_bord[3*(i + (h-1)*w)+c] = pixels[3*(i + (h-1)*w)+c];
        }
        for ( j = 1; j < h; ++j )
        {
            image_bord[3*(j*w)+c] = pixels[3*j*w+c];
            image_bord[3*((j+1)*w-1)+c] = pixels[3*((j+1)*w-1)+c];
        }
        // Puis calcul du bord de l'image 
        // On dégrade à l'ordre 1 au bord de l'image :
        i = 1;
        for ( j = 1; j <= height; ++j )
        {
            std::uint32_t ind = 3*(i+j*w)+c;
            image_bord[ind] = (4*pixels[ind] - pixels[ind-3] - pixels[ind+3] - pixels[ind - 3*w] - pixels[ind + 3*w]);
            if (image_bord[ind] < 0.)
                image_bord[ind] = 0.;
            if (image_bord[ind] > 255.)   
                image_bord[ind] = 255.;
        }
        i = width;
        for ( j = 1; j <= height; ++j )
        {
            std::uint32_t ind = 3*(i+j*w)+c;
            image_bord[ind] = (4*pixels[ind] - pixels[ind-3] - pixels[ind+3] - pixels[ind - 3*w] - pixels[ind + 3*w]);
            if (image_bord[ind] < 0.)
                image_bord[ind] = 0.;
            if (image_bord[ind] > 255.)   
                image_bord[ind] = 255.;
        }
        j = 1;
        for (i = 1; i <= width; ++i )
        {
            std::uint32_t ind = 3*(i+j*w)+c;
            image_bord[ind] = (4*pixels[ind] - pixels[ind-3] - pixels[ind+3] - pixels[ind - 3*w] - pixels[ind + 3*w]);
            if (image_bord[ind] < 0.)
                image_bord[ind] = 0.;
            if (image_bord[ind] > 255.)   
                image_bord[ind] = 255.;
        }
        j = height;
        for (i = 1; i <= width; ++i )
        {
            std::uint32_t ind = 3*(i+j*w)+c;
            image_bord[ind] = (4*pixels[ind] - pixels[ind-3] - pixels[ind+3] - pixels[ind - 3*w] - pixels[ind + 3*w]);
            if (image_bord[ind] < 0.)
                image_bord[ind] = 0.;
            if (image_bord[ind] > 255.)   
                image_bord[ind] = 255.;
        }
        int dx = 3, dy = 3*w;
        for ( std::uint32_t i = 2; i < width; ++i )
        {
            for ( std::uint32_t j = 2; j < height; ++j )
            {
                std::uint32_t ind = 3*(i+j*w)+c;
                image_bord[ind] = (
                -pixels[ind-2*dy]
                -pixels[ind-dx-dy] -2*pixels[ind-dy] - pixels[ind+dx-dy] 
                -pixels[ind-2*dx] - 2*pixels[ind-dx] + 16*pixels[ind] - 2*pixels[ind+dx] - pixels[ind+2*dx] 
                -pixels[ind-dx+dy] -2*pixels[ind+dy] - pixels[ind+dx+dy] 
                -pixels[ind+2*dy]);
                if (image_bord[ind] < 0.)
                    image_bord[ind] = 0.;
                if (image_bord[ind] > 255.)   
                    image_bord[ind] = 255.;
            }
        }
    }
    return image_bord;
}


std::vector<double> rehausseur( std::uint32_t width, std::uint32_t height, std::vector<double> const& pixels )
{
    std::uint32_t w = width  + 2;
    std::uint32_t h = height + 2;
    assert(pixels.size() == 3*w*h);
    std::vector<double> image_nette(3*w*h);
    for ( int c = 0; c < 3; ++c )
    {
        // Copie pour les bords (condition limite) :
        for ( std::uint32_t i = 0; i < w; ++i )
        {
            image_nette[3*i+c] = pixels[3*i+c];
            image_nette[3*(i + (h-1)*w)+c] = pixels[3*(i + (h-1)*w)+c];
        }
        for ( std::uint32_t j = 1; j < h; ++j )
        {
            image_nette[3*(j*w)+c] = pixels[3*j*w+c];
            image_nette[3*((j+1)*w-1)+c] = pixels[3*((j+1)*w-1)+c];
        }
        int dx = 3, dy = 3*w;
        // Puis rehausseur de l'image :
        for ( std::uint32_t i = 1; i <= width; ++i )
        {
            for ( std::uint32_t j = 1; j <= height; ++j )
            {
                std::uint32_t ind = 3*(i+j*w)+c;
                image_nette[ind] =
                   -1./6. * pixels[ind - dy - dx] - 2. /3.*pixels[ind - dy] - 1./6.*pixels[ind - dy + dx]
                   -2./3. * pixels[ind      - dx] + 13./3.*pixels[ind     ] - 2./3.*pixels[ind      + dx]
                   -1./6. * pixels[ind + dy - dx] - 2. /3.*pixels[ind + dy] - 1./6.*pixels[ind + dy + dx];
                                    
                if (image_nette[ind] < 0.)
                    image_nette[ind] = 0.;
                if (image_nette[ind] > 255.)   
                    image_nette[ind] = 255.;
            }
        }
    }
    return image_nette;
}

int main(int nargs, char* argv[])
{
    std::vector<std::string> strChoix = {
        "dessin",
        "bord ordre 2",
        "hausseur nettete",
        "flou gaussien",
        "Inversion couleur"
    };
    if (nargs < 2 )
    {
        std::cout << R"RAW(
Utilisation : ./filtres_images.exe choix [image]
                              ou
              choix = 1 : Utilisation du filtre laplacien ordre 1
              choix = 2 : Utilisation du filtre laplacien ordre 2
              choix = 3 : Utilisation filtre rehausseur de netteté
              choix = 4 : Utilisation du filtre gaussien
              choix = 5 : Utilisation du filtre inversion couleur

              image est optionnelle. Par defaut, prend la photo de lena

Exemple :
    ./filtres_images.exe 1 ./data/pexels-arthouse-studio-4311512.png
Utilise le filtre laplacien sur l'image pexels-arthouse-studio-4311512.png
        )RAW";
        return EXIT_FAILURE;
    }

    int choix = std::stoi(argv[1]) ;
    if ( (choix < 1) || (choix > int(strChoix.size())) )
    {
        std::cout << "Le numero du filtre doit etre compris entre 1 et " << strChoix.size() << std::endl;
    }

    std::string image_file = "data/lena_gray.png";
    if (nargs > 2)
        image_file = std::string(argv[2]);

    std::uint32_t width, height;
    std::uint8_t* image;
    // Chargement de l'image
    std::cout << "Chargement de l'image " << image_file << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    auto error = lodepng_decode24_file(&image, &width, &height, image_file.c_str());
    auto end   = std::chrono::high_resolution_clock::now();
    if(error) std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    std::cout << "Filtre choisi : " << strChoix[choix-1] << std::endl;
    std::cout << "Image traitee de taille " << width << " x " << height << std::endl;
    std::cout << "Temps chargement de l'image : " << std::chrono::duration<double, std::milli>(end - start).count()
              << "ms" << std::endl;


    // Normalisation de l'image + remplissage des pixels fantômes
    auto start1 = std::chrono::high_resolution_clock::now();
    auto image_etendue = normaliseEtEtendre(width, height, image);
    end   = std::chrono::high_resolution_clock::now();
    std::cout << "Temps normalisation et extension de l'image : " << std::chrono::duration<double, std::milli>(end - start1).count()
              << "ms" << std::endl; 

    std::vector<double>  image_filtree_etendue;
    if (choix == 1)
    {
        start = std::chrono::high_resolution_clock::now();
        auto image_bord = bord(width, height, image_etendue);
        end   = std::chrono::high_resolution_clock::now();
        std::cout << "Temps détection des bords de l'image : " << std::chrono::duration<double, std::milli>(end - start).count()
                  << "ms" << std::endl; 

        start = std::chrono::high_resolution_clock::now();
        image_filtree_etendue = negative(width, height, image_bord);
        end   = std::chrono::high_resolution_clock::now();
        std::cout << "Temps inversion couleurs de l'image : " << std::chrono::duration<double, std::milli>(end - start).count()
                  << "ms" << std::endl; 
    }
    else if (choix == 2)
    {
        start = std::chrono::high_resolution_clock::now();
        auto image_flou = flou_gaussien(width, height, image_etendue);
        end   = std::chrono::high_resolution_clock::now();
        std::cout << "Temps détection flou gaussien de l'image : " << std::chrono::duration<double, std::milli>(end - start).count()
                  << "ms" << std::endl; 
        start = std::chrono::high_resolution_clock::now();
        auto image_bord = bord2(width, height, image_flou);
        end   = std::chrono::high_resolution_clock::now();
        std::cout << "Temps détection des bords de l'image : " << std::chrono::duration<double, std::milli>(end - start).count()
                  << "ms" << std::endl; 
        start = std::chrono::high_resolution_clock::now();
        image_filtree_etendue = negative(width, height, image_bord);
        end   = std::chrono::high_resolution_clock::now();
        std::cout << "Temps inversion couleurs de l'image : " << std::chrono::duration<double, std::milli>(end - start).count()
                  << "ms" << std::endl; 
    }
    else if (choix == 3)
    {
        start = std::chrono::high_resolution_clock::now();
        image_filtree_etendue = rehausseur( width, height, image_etendue );
        end   = std::chrono::high_resolution_clock::now();
        std::cout << "Temps rehaussage nettete de l'image : " << std::chrono::duration<double, std::milli>(end - start).count()
                  << "ms" << std::endl; 
    }
    else if (choix == 4)
    {
        start = std::chrono::high_resolution_clock::now();
        image_filtree_etendue = flou_gaussien(width, height, image_etendue);
        end   = std::chrono::high_resolution_clock::now();
        std::cout << "Temps détection flou gaussien de l'image : " << std::chrono::duration<double, std::milli>(end - start).count()
                  << "ms" << std::endl; 
    }
    else if (choix == 5)
    {
        start = std::chrono::high_resolution_clock::now();
        image_filtree_etendue = negative(width, height, image_etendue);
        end   = std::chrono::high_resolution_clock::now();
        std::cout << "Temps inversion couleurs de l'image : " << std::chrono::duration<double, std::milli>(end - start).count()
                  << "ms" << std::endl; 
    }

        
    start = std::chrono::system_clock::now();
    auto image_filtree = transformeImage8Bits(width, height, image_filtree_etendue);
    end   = std::chrono::system_clock::now();
    std::cout << "Temps transformation image 24 bits : " << std::chrono::duration<double, std::milli>(end - start).count()
              << "ms" << std::endl; 
    std::cout << "Temps transformation image en dessin (cumul) : " << std::chrono::duration<double, std::milli>(end - start1).count()
              << "ms" << std::endl; 

    start = std::chrono::system_clock::now();
    error = lodepng_encode24_file("image_filtree.png", image_filtree.data(), width, height);
    end   = std::chrono::system_clock::now();
    std::cout << "Temps sauvegarde image : " << std::chrono::duration<double, std::milli>(end - start).count()
              << "ms" << std::endl; 

    if(error) std::cerr << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    delete [] image;

    return EXIT_SUCCESS;
}