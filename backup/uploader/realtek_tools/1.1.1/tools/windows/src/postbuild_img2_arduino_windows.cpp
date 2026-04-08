/*

Compile:

windows:
mingw32-g++.exe -o postbuild_img2_arduino_windows.exe tools\windows\src\postbuild_img2_arduino_windows.cpp -static

*/

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <vector>

using namespace std;

void replaceAll( string& source, const string& from, const string& to )
{
    string newString;
    newString.reserve(source.length()); //avoids a few memory allocations

    string::size_type lastPos = 0;
    string::size_type findPos;

    while (string::npos != (findPos = source.find(from, lastPos))) {
        newString.append( source, lastPos, (findPos - lastPos));
        newString += to;
        lastPos = findPos + from.length();
    }

    // Care for the rest after last occurrence
    newString += source.substr(lastPos);

    source.swap(newString);
}

int main(int argc, char *argv[]) {

    int ret = 0;
    stringstream cmdss;
    string cmd, line, msg;
    vector<string> lines;
    vector<string>::iterator iter;
    string path_tool;
    string path_arm_none_eabi_gcc;
    string path_symbol_black_list;
    string bksym;
    ifstream fin;
    ofstream fout;

    bool has_xip = false;
    bool has_psram = false;

    string sram_start_st = "", sram_end_st = "", xip_start_st = "", xip_end_st = "", psram_start_st = "", psram_end_st = "";
    unsigned int sram_start = 0, sram_end = 0, xip_start = 0, xip_end = 0, psram_start = 0, psram_end = 0;

    size_t pos;

    int check_powersavemode = 0;

    // 0. change work folder
    chdir(argv[1]);

    // 1. copy elf application.axf to current folder
    cmdss.clear();
    cmdss << "xcopy /y " << argv[2] << " .\\";
    getline(cmdss, cmd);
    cout << cmd << endl;
    system(cmd.c_str());

    // 2. remove previous files
    cmd = "if exist application.map del application.map";
    cout << cmd << endl;
    system(cmd.c_str());

    cmd = "if exist application.asm del application.asm";
    cout << cmd << endl;
    system(cmd.c_str());

    cmd = "if exist *.bin del *.bin";
    cout << cmd << endl;
    system(cmd.c_str());

    cmd = "copy tools\\windows\\image_tool\\imgtool_flashloader_amebad.bin .\\";
    cout << cmd << endl;
    system(cmd.c_str());

    // 3. generate information files
    path_arm_none_eabi_gcc.assign(argv[3]);
    replaceAll(path_arm_none_eabi_gcc, "/", "\\");

    cmdss.clear();
    cmdss << "\"" <<path_arm_none_eabi_gcc << "arm-none-eabi-nm.exe\" application.axf | sort > application.map";
    getline(cmdss, cmd);
    cout << cmd << endl;
    system(cmd.c_str());

    fin.open("application.map");
    while (getline(fin, line)) {
        lines.push_back(line);
    }
    fin.close();

    cmdss.clear();
    cmdss << "\"" <<path_arm_none_eabi_gcc << "arm-none-eabi-objdump.exe\" -d application.axf > application.asm";
    getline(cmdss, cmd);
    cout << cmd << endl;
    system(cmd.c_str());

    // 3.1 check if any forbidden symbols
    path_symbol_black_list.assign(argv[4]);
    replaceAll(path_symbol_black_list, "/", "\\");
    fin.open(path_symbol_black_list.c_str(), ifstream::in);
    cout << path_symbol_black_list << endl;
    ret = 0;
    if (fin) {
        while (!fin.eof() && ret == 0) {
            fin >> bksym;
            getline(fin, msg);

            // check if this symbole appears in the map file
            for (iter = lines.begin(); iter != lines.end(); ++iter) {
                if ((iter->find(bksym)) != string::npos) {
                    cerr << "ERROR: " << msg << endl;
                    ret = -1;
                    break;
                }
            }
        }
    }
    fin.close();

    if (ret != 0) {
        return -1;
    }

    // 4. grep sram, xip/flash and psram information
    fout.open("application.map");
    for (iter = lines.begin(); iter != lines.end(); ++iter) {
        fout << *iter << endl;
        line = *iter;
        pos = line.find("__ram_image2_text_start__");
        if (pos != string::npos) {
            sram_start_st = line.substr(0, (pos - 3));
            sram_start = strtol(sram_start_st.c_str(), NULL, 16);
        }
        pos = line.find("__ram_image2_text_end__");
        if (pos != string::npos) {
            sram_end_st = line.substr(0, (pos - 3));
            sram_end = strtol(sram_end_st.c_str(), NULL, 16);
        }

        pos = line.find("__flash_text_start__");
        if (pos != string::npos) {
            xip_start_st = line.substr(0, (pos - 3));
            xip_start = strtol(xip_start_st.c_str(), NULL, 16);
        }
        pos = line.find("__flash_text_end__");
        if (pos != string::npos) {
            xip_end_st = line.substr(0, (pos - 3));
            xip_end = strtol(xip_end_st.c_str(), NULL, 16);
        }

        pos = line.find("__psram_image2_text_start__");
        if (pos != string::npos) {
            psram_start_st = line.substr(0, (pos - 3));
            psram_start = strtol(psram_start_st.c_str(), NULL, 16);
        }
        pos = line.find("__psram_image2_text_end__");
        if (pos != string::npos) {
            psram_end_st = line.substr(0, (pos - 3));
            psram_end = strtol(psram_end_st.c_str(), NULL, 16);
        }

        //4.1 power save bin handling
        //DS_AON_TIMER_WAKEUP               101
        //DS_RTC_WAKEUP                     102

        //TL_UART_WAKEUP                    201
        //TL_RTC_WAKEUP                     202

        //AON_WAKEPIN_WAKEUP_PA12           312
        //AON_WAKEPIN_WAKEUP_PA13           313
        //AON_WAKEPIN_WAKEUP_PA14           314
        //AON_WAKEPIN_WAKEUP_PA15           315
        //AON_WAKEPIN_WAKEUP_PA16           316
        //AON_WAKEPIN_WAKEUP_PA17           317
        //AON_WAKEPIN_WAKEUP_PA18           318
        //AON_WAKEPIN_WAKEUP_PA19           319
        //AON_WAKEPIN_WAKEUP_PA20           320
        //AON_WAKEPIN_WAKEUP_PA21           321
        //AON_WAKEPIN_WAKEUP_PA25           325
        //AON_WAKEPIN_WAKEUP_PA26           326

        pos = line.find("DS_AON_TIMER_WAKEUP");
        if (pos != string::npos) {
            check_powersavemode = 101;
        }

        pos = line.find("DS_RTC_WAKEUP");
        if (pos != string::npos) {
            check_powersavemode = 102;
        }

        pos = line.find("TL_UART_WAKEUP");
        if (pos != string::npos) {
            check_powersavemode = 201;
        }

        pos = line.find("TL_RTC_WAKEUP");
        if (pos != string::npos) {
            check_powersavemode = 202;
        }

        pos = line.find("AON_WAKEPIN_WAKEUP_PA12");
        if (pos != string::npos) {
            check_powersavemode = 312;
        }

        pos = line.find("AON_WAKEPIN_WAKEUP_PA13");
        if (pos != string::npos) {
            check_powersavemode = 313;
        }

        pos = line.find("AON_WAKEPIN_WAKEUP_PA14");
        if (pos != string::npos) {
            check_powersavemode = 314;
        }

        pos = line.find("AON_WAKEPIN_WAKEUP_PA15");
        if (pos != string::npos) {
            check_powersavemode = 315;
        }

        pos = line.find("AON_WAKEPIN_WAKEUP_PA16");
        if (pos != string::npos) {
            check_powersavemode = 316;
        }

        pos = line.find("AON_WAKEPIN_WAKEUP_PA17");
        if (pos != string::npos) {
            check_powersavemode = 317;
        }

        pos = line.find("AON_WAKEPIN_WAKEUP_PA18");
        if (pos != string::npos) {
            check_powersavemode = 318;
        }

        pos = line.find("AON_WAKEPIN_WAKEUP_PA19");
        if (pos != string::npos) {
            check_powersavemode = 319;
        }

        pos = line.find("AON_WAKEPIN_WAKEUP_PA20");
        if (pos != string::npos) {
            check_powersavemode = 320;
        }

        pos = line.find("AON_WAKEPIN_WAKEUP_PA21");
        if (pos != string::npos) {
            check_powersavemode = 321;
        }

        pos = line.find("AON_WAKEPIN_WAKEUP_PA25");
        if (pos != string::npos) {
            check_powersavemode = 325;
        }

        pos = line.find("AON_WAKEPIN_WAKEUP_PA26");
        if (pos != string::npos) {
            check_powersavemode = 326;
        }
    }
    fout.close();

    if (xip_start > 0 && xip_end > 0) {
        has_xip = true;
    }
    if (psram_start > 0 && psram_end > 0) {
        has_psram = true;
    }

    cout << "sram  " << sram_start_st << " ~ " << sram_end_st << endl;
    if (has_xip) {
        cout << "xip " << xip_start_st << " ~ " << xip_end_st << endl;
    }
    if (has_psram) {
        cout << "psram " << psram_start_st << " ~ " << psram_end_st << endl;
    }

    // 5. generate image 2, image xip and image psram
    cmdss.clear();
    //cmdss << "\"" <<path_arm_none_eabi_gcc << "arm-none-eabi-objcopy.exe\" -j .image2.start.table -j .ram_image2.text -j .ram.data -Obinary .\\application.axf .\\ram_2.bin";
    cmdss << "\"" <<path_arm_none_eabi_gcc << "arm-none-eabi-objcopy.exe\" -j .ram_image2.entry -j .ram_image2.text -j .ram_image2.data -Obinary .\\application.axf .\\ram_2.r.bin";
    getline(cmdss, cmd);
    cout << cmd << endl;
    system(cmd.c_str());

    if (has_xip) {
        cmdss.clear();
        //cmdss << "\"" << path_arm_none_eabi_gcc << "arm-none-eabi-objcopy.exe\" -j .image3 -j .sdr_data -Obinary .\\application.axf .\\xip_image2.bin";
        cmdss << "\"" << path_arm_none_eabi_gcc << "arm-none-eabi-objcopy.exe\" -j .xip_image2.text -Obinary .\\application.axf .\\xip_image2.bin";
        getline(cmdss, cmd);
        cout << cmd << endl;
        system(cmd.c_str());
    }

    if (has_psram) {
        cmdss.clear();
        //cmdss << "\"" << path_arm_none_eabi_gcc << "arm-none-eabi-objcopy.exe\" -j .image3 -j .sdr_data -Obinary .\\application.axf .\\psram_2.bin";
        cmdss << "\"" << path_arm_none_eabi_gcc << "arm-none-eabi-objcopy.exe\" -j .psram_image2.text -j .psram_image2.data -Obinary .\\application.axf .\\psram_2.r.bin";
        getline(cmdss, cmd);
        cout << cmd << endl;
        system(cmd.c_str());
    }


    // 6. fulfill header
    // 6.1 remove bss sections
    cmdss.clear();
    cmdss << ".\\tools\\windows\\pick.exe " << sram_start << " " << sram_end << " ram_2.r.bin ram_2.bin raw";
    getline(cmdss, cmd);
    cout << cmd << endl;
    system(cmd.c_str());

    if (has_psram) {
        cmdss.clear();
        cmdss << ".\\tools\\windows\\pick.exe " << psram_start << " " << psram_end << " psram_2.r.bin psram_2.bin raw";
        getline(cmdss, cmd);
        cout << cmd << endl;
        system(cmd.c_str());
    }

    // 6.2 add header
    cmdss.clear();
    cmdss << ".\\tools\\windows\\pick.exe " << sram_start << " " << sram_end << " ram_2.bin ram_2.p.bin";
    getline(cmdss, cmd);
    cout << cmd << endl;
    system(cmd.c_str());

    if (has_xip) {
        cmdss.clear();
        cmdss << ".\\tools\\windows\\pick.exe " << xip_start << " " << xip_end << " xip_image2.bin xip_image2.p.bin";
        getline(cmdss, cmd);
        cout << cmd << endl;
        system(cmd.c_str());
    }

    if (has_psram) {
        cmdss.clear();
        cmdss << ".\\tools\\windows\\pick.exe " << psram_start << " " << psram_end << " psram_2.bin psram_2.p.bin";
        getline(cmdss, cmd);
        cout << cmd << endl;
        system(cmd.c_str());
    }

    // 6.3 generate km4_image2_all
    cmd = "copy /b xip_image2.p.bin+ram_2.p.bin+psram_2.p.bin km4_image2_all.bin";
    cout << cmd << endl;
    system(cmd.c_str());

    cmdss.clear();
    cmdss << ".\\tools\\windows\\pad.exe " << "km4_image2_all.bin 4096";
    getline(cmdss, cmd);
    cout << cmd << endl;
    system(cmd.c_str());


    // 7. prepare image 1
    // 7.1 power save mode bins
    //DS_AON_TIMER_WAKEUP               101
    //DS_RTC_WAKEUP                     102

    //TL_UART_WAKEUP                    201
    //TL_RTC_WAKEUP                     202

    //AON_WAKEPIN_WAKEUP_PA12           312
    //AON_WAKEPIN_WAKEUP_PA13           313
    //AON_WAKEPIN_WAKEUP_PA14           314
    //AON_WAKEPIN_WAKEUP_PA15           315
    //AON_WAKEPIN_WAKEUP_PA16           316
    //AON_WAKEPIN_WAKEUP_PA17           317
    //AON_WAKEPIN_WAKEUP_PA18           318
    //AON_WAKEPIN_WAKEUP_PA19           319
    //AON_WAKEPIN_WAKEUP_PA20           320
    //AON_WAKEPIN_WAKEUP_PA21           321
    //AON_WAKEPIN_WAKEUP_PA25           325
    //AON_WAKEPIN_WAKEUP_PA26           326

    cmd = "if exist bsp\\image*.bin del bsp\\image*.bin";
    cout << cmd << endl;
    system(cmd.c_str());

    if (check_powersavemode == 101) {
        //cmd = "copy bsp\\image\\PMU_bins\\NONE\\km0_boot_all.bin bsp\\image";
        //cout << cmd << endl;
        //system(cmd.c_str());
        //cmd = "copy bsp\\image\\PMU_bins\\NONE\\km4_boot_all.bin bsp\\image";
        //cout << cmd << endl;
        //system(cmd.c_str());
        cmd = "copy bsp\\image\\PMU_bins\\NONE\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 102) {
        //cmd = "copy bsp\\image\\PMU_bins\\DS_RTC\\km0_boot_all.bin bsp\\image";
        //cout << cmd << endl;
        //system(cmd.c_str());
        //cmd = "copy bsp\\image\\PMU_bins\\DS_RTC\\km4_boot_all.bin bsp\\image";
        //cout << cmd << endl;
        //system(cmd.c_str());
        cmd = "copy bsp\\image\\PMU_bins\\DS_RTC\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 201) {
        //cmd = "copy bsp\\image\\PMU_bins\\TL_UART\\km0_boot_all.bin bsp\\image";
        //cout << cmd << endl;
        //system(cmd.c_str());
        //cmd = "copy bsp\\image\\PMU_bins\\TL_UART\\km4_boot_all.bin bsp\\image";
        //cout << cmd << endl;
        //system(cmd.c_str());
        cmd = "copy bsp\\image\\PMU_bins\\TL_UART\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 202) {
        //cmd = "copy bsp\\image\\PMU_bins\\TL_RTC\\km0_boot_all.bin bsp\\image";
        //cout << cmd << endl;
        //system(cmd.c_str());
        //cmd = "copy bsp\\image\\PMU_bins\\TL_RTC\\km4_boot_all.bin bsp\\image";
        //cout << cmd << endl;
        //system(cmd.c_str());
        cmd = "copy bsp\\image\\PMU_bins\\TL_RTC\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 312) {
        cmd = "copy bsp\\image\\PMU_bins\\PA12\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 313) {
        cmd = "copy bsp\\image\\PMU_bins\\PA13\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 314) {
        cmd = "copy bsp\\image\\PMU_bins\\PA14\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 315) {
        cmd = "copy bsp\\image\\PMU_bins\\PA15\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 316) {
        cmd = "copy bsp\\image\\PMU_bins\\PA16\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 317) {
        cmd = "copy bsp\\image\\PMU_bins\\PA17\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 318) {
        cmd = "copy bsp\\image\\PMU_bins\\PA18\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 319) {
        cmd = "copy bsp\\image\\PMU_bins\\PA19\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 320) {
        cmd = "copy bsp\\image\\PMU_bins\\PA20\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 321) {
        cmd = "copy bsp\\image\\PMU_bins\\PA21\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 325) {
        cmd = "copy bsp\\image\\PMU_bins\\PA25\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else if (check_powersavemode == 326) {
        cmd = "copy bsp\\image\\PMU_bins\\PA26\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    } else {
        //cmd = "copy bsp\\image\\PMU_bins\\NONE\\km0_boot_all.bin bsp\\image";
        //cout << cmd << endl;
        //system(cmd.c_str());
        //cmd = "copy bsp\\image\\PMU_bins\\NONE\\km4_boot_all.bin bsp\\image";
        //cout << cmd << endl;
        //system(cmd.c_str());
        cmd = "copy bsp\\image\\PMU_bins\\NONE\\km0_image2_all.bin bsp\\image";
        cout << cmd << endl;
        system(cmd.c_str());
    }

    check_powersavemode = 0;

    cmd = "copy bsp\\image\\PMU_bins\\NONE\\km0_boot_all.bin .\\";
    cout << cmd << endl;
    system(cmd.c_str());

    cmd = "copy bsp\\image\\PMU_bins\\NONE\\km4_boot_all.bin .\\";
    cout << cmd << endl;
    system(cmd.c_str());

    cmd = "copy bsp\\image\\km0_image2_all.bin .\\";
    cout << cmd << endl;
    system(cmd.c_str());

    // 8. generate .bin
    cmd = "copy /b km0_image2_all.bin+km4_image2_all.bin km0_km4_image2.bin";
    cout << cmd << endl;
    system(cmd.c_str());

#if 0
    // 9. add checksum
    cmd = ".\\tools\\windows\\checksum.exe ota.bin";
    cout << cmd << endl;
    system(cmd.c_str());
#endif

    return 0;
}
