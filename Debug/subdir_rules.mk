################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-arm_18.12.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/FHG/Desktop/httpget_TIVA4C" --include_path="C:/ti/ccs920/tirtos_tivac_2_16_01_14/products/ndk_2_25_00_09/packages/ti/ndk/inc/bsd" --include_path="C:/Users/FHG/Desktop/httpget_TIVA4C" --include_path="C:/ti/ccs920/tirtos_tivac_2_16_01_14/products/TivaWare_C_Series-2.1.1.71b" --include_path="C:/ti/ccs920/tirtos_tivac_2_16_01_14/products/bios_6_45_02_31/packages/ti/sysbios/posix" --include_path="C:/ti/ccs920/ccs/tools/compiler/ti-cgt-arm_18.12.3.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=NET_NDK --define=ccs --define=TIVAWARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1943891247:
	@$(MAKE) --no-print-directory -Onone -f subdir_rules.mk build-1943891247-inproc

build-1943891247-inproc: ../httpget.cfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: XDCtools'
	"C:/ti/xdctools_3_32_00_06_core/xs" --xdcpath="C:/ti/ccs920/tirtos_tivac_2_16_01_14/packages;C:/ti/ccs920/tirtos_tivac_2_16_01_14/products/tidrivers_tivac_2_16_01_13/packages;C:/ti/ccs920/tirtos_tivac_2_16_01_14/products/bios_6_45_02_31/packages;C:/ti/ccs920/tirtos_tivac_2_16_01_14/products/ndk_2_25_00_09/packages;C:/ti/ccs920/tirtos_tivac_2_16_01_14/products/uia_2_00_05_50/packages;C:/ti/ccs920/tirtos_tivac_2_16_01_14/products/ns_1_11_00_10/packages;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M4F -p ti.platforms.tiva:TM4C1294NCPDT -r release -c "C:/ti/ccs920/ccs/tools/compiler/ti-cgt-arm_18.12.3.LTS" --compileOptions "-mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path=\"C:/Users/FHG/Desktop/httpget_TIVA4C\" --include_path=\"C:/ti/ccs920/tirtos_tivac_2_16_01_14/products/ndk_2_25_00_09/packages/ti/ndk/inc/bsd\" --include_path=\"C:/Users/FHG/Desktop/httpget_TIVA4C\" --include_path=\"C:/ti/ccs920/tirtos_tivac_2_16_01_14/products/TivaWare_C_Series-2.1.1.71b\" --include_path=\"C:/ti/ccs920/tirtos_tivac_2_16_01_14/products/bios_6_45_02_31/packages/ti/sysbios/posix\" --include_path=\"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-arm_18.12.3.LTS/include\" --define=ccs=\"ccs\" --define=PART_TM4C1294NCPDT --define=NET_NDK --define=ccs --define=TIVAWARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi  " "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

configPkg/linker.cmd: build-1943891247 ../httpget.cfg
configPkg/compiler.opt: build-1943891247
configPkg/: build-1943891247


