#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -eo pipefail

# Enable alias expansion within non-interactive bash scripts
shopt -s expand_aliases

# Source alias definitions from bashrc/bash_profile if available
if [ -f ~/.bashrc ]; then
    source ~/.bashrc
elif [ -f ~/.bash_profile ]; then
    source ~/.bash_profile
fi
set -u
# --- SQL Script Definition ---
SQL_SCRIPT=$(cat <<'EOF'
USE hamoller_db;

-- 1. Run_info Table
CREATE TABLE IF NOT EXISTS Run_info (
    run_number 	    	   INT UNSIGNED PRIMARY KEY,
    run_group 	    	   INT UNSIGNED,
    run_experiment  	   VARCHAR(255) COMMENT 'Name of experiment',
    run_start 		   VARCHAR(50) COMMENT 'Start of run time stamp', -- Fits Linux 'date' default string (e.g., "Fri Jul 31 16:54:21 EDT 2026")
    run_end 		   VARCHAR(50)COMMENT 'End of run time stamp',
    measurement_type 	   ENUM(
        		   'Rate scan',
        		   'Polarization',
        		   'Systematic study',
        		   'Bleedthrough',
        		   'Gain matching',
        		   'Threshold check',
        		   'Other') NOT NULL DEFAULT 'Other',
    run_quality		   ENUM(
    			   'Good', 
    			   'Bad', 
    			   'Suspect',
    			   'Undetermined') NOT NULL DEFAULT 'Undetermined',
    beam_sigma_x 	   FLOAT(10,5) COMMENT '1 sigma x-width (mm) of beam from harp scan',
    beam_sigma_y 	   FLOAT(10,5) COMMENT '1 sigma y-width (mm) of beam from harp scan',
    requested_current 	   FLOAT(10,5) COMMENT 'Requested beam current in microamperes',
    target_pol 		   FLOAT(10,8) COMMENT 'Calculated target polarization',
    target_foil_avgT 	   FLOAT(10,5) COMMENT 'Foil temperature in Kelvin weighted by beam intensity',
    hallA_ambientT 	   FLOAT(10,5) COMMENT 'Hall A ambient temperature in Kelvin',
    run_qped 		   SMALLINT COMMENT 'Charge pedestal in counts',
    run_deadtime_tau_1 	   DOUBLE COMMENT 'Dead time constant (ns) for coinc-coinc pile up',
    run_deadtime_tau_2 	   DOUBLE COMMENT 'Dead time constant (ns) for single-coinc pile up',
    run_accid_tau 	   DOUBLE COMMENT 'Accidental window width (ns)',	       
    comment 		   TEXT,
    last_updated 	   TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP 
        		   COMMENT 'Timestamp of last record update'
) ENGINE=InnoDB;

-- 2. DAQ_config Table
CREATE TABLE IF NOT EXISTS DAQ_config (
    run_number INT UNSIGNED PRIMARY KEY,
    
    -- Trigger Prescales
    prescale0 INT,
    prescale1 INT,
    prescale2 INT,
    prescale3 INT,
    prescale4 INT,
    prescale5 INT,
    prescale6 INT,
    
-- Crate & Slot Identification
    FADC_CRATE		  VARCHAR(255) DEFAULT NULL COMMENT 'Crate identifier or hostname (e.g., all, hapolmollervme.jlab.org)',
    FADC_SLOT 		  VARCHAR(255) DEFAULT NULL COMMENT 'Slot selection (e.g., all, 3)',

    -- Channel Masks & Operating Modes
    FADC_ADC_MASK      	  VARCHAR(255) DEFAULT NULL COMMENT 'ADC channel enable mask',
    FADC_TRG_MASK 	  VARCHAR(255) DEFAULT NULL COMMENT 'Trigger channel enable mask',
    FADC_TET_IGNORE_MASK  VARCHAR(255) DEFAULT NULL COMMENT 'Force readout of channel mask (i.e. ignore threshold for readout)',
    FADC_ALLCH_MODE 	  VARCHAR(255) DEFAULT NULL COMMENT 'Set the FADC mode for each channel',

    -- Windowing & Timing Definitions
    FADC_W_OFFSET	  SMALLINT UNSIGNED DEFAULT NULL COMMENT 'Number of ns back from trigger point',
    FADC_W_WIDTH 	  SMALLINT UNSIGNED DEFAULT NULL COMMENT 'Number of ns to include in trigger window',
    FADC_NSB 		  SMALLINT UNSIGNED DEFAULT NULL COMMENT 'Time (units: ns) before threshold crossing to include in integral',
    FADC_NSA 		  SMALLINT UNSIGNED DEFAULT NULL COMMENT 'Time (units: ns) after threshold crossing to include in integral',

    -- Peak Processing & Pedestal Limits
    FADC_NPEAK 	       	  SMALLINT UNSIGNED DEFAULT NULL COMMENT 'Number of Pulses allowed for each window',
    FADC_NPED 		  SMALLINT UNSIGNED DEFAULT NULL COMMENT 'Number of samples included in the pedestal sum',
    FADC_MAXPED 	  SMALLINT UNSIGNED DEFAULT NULL COMMENT 'Maximum value of sample to be included in pedestal sum (0--1023)',
    FADC_NSAT 		  SMALLINT UNSIGNED DEFAULT NULL COMMENT 'Number of consecutive samples over threshold for valid pulse (1--4)',
    FADC_TET 		  SMALLINT UNSIGNED DEFAULT NULL COMMENT 'Fadc channel hit threshold (adc channel). May be overridden by individual channel TETs',

    -- DAC, Gain & Accumulator Configuration
    FADC_DAC				SMALLINT UNSIGNED DEFAULT NULL COMMENT 'Board DAC, one and the same for all 16 channels (DAC/mV)',
    FADC_GAIN 		  		FLOAT(10,5) DEFAULT NULL COMMENT 'Board Gains, same for all channels (MeV/channel)',
    FADC_ACCUMULATOR_SCALER_MODE_MASK 	VARCHAR(255) DEFAULT NULL COMMENT 'Accumulator scaler mode: 0=Default, TET based pulse integration, 1=Sum all samples',

    -- Møller Discriminator & Trigger Logic Settings
    FADC_MOLLER 	      VARCHAR(255) DEFAULT NULL COMMENT 'L_OFFSET, R_OFFSET, L_SUM_THR, R_SUM_THR, DISC_WIDTH, DISC_MODE parameters',
    FADC_L_OFFSET 	      FLOAT(10,5) COMMENT 'ADC amplitude subtracted from the sum of the 4 left channels i.e. the sum pedestal',
    FADC_R_OFFSET 	      FLOAT(10,5) COMMENT 'ADC amplitude subtracted from the sum of the 4 right channels i.e. the sum pedestal',
    FADC_DISC_WIDTH 	      FLOAT(10,5) COMMENT 'Coincidence with in 4ns units (the left/right sum discriminator width)',
    FADC_DISC_MODE 	      TINYINT(1) COMMENT 'When 0 the left/right sum discriminators are operating in non-updating mode',
    FADC_L_SUM_THR 	      FLOAT(10,5) COMMENT 'Threshold which the left sum must pass',
    FADC_R_SUM_THR 	      FLOAT(10,5) COMMENT 'Threshold which the right sum must pass',
    FADC_TRG_SEL 	      TINYINT(1) DEFAULT NULL COMMENT '0=multiplicity, 1=moller-AND, 2=moller-OR',
    FADC_TRG_WIDTH 	      SMALLINT UNSIGNED DEFAULT NULL COMMENT 'Stretches pulse width of channel over threshold in 4ns ticks for TI input',

    -- Pedestal & Channel Threshold Overrides
    FADC_ALLCH_PED 	      TEXT DEFAULT NULL COMMENT 'Pedestal values for all 16 channels',
    FADC_CH_PED    	      VARCHAR(255) DEFAULT NULL COMMENT 'Specific channel pedestal override setting',
    FADC_CH_TET 	      VARCHAR(255) DEFAULT NULL COMMENT 'Specific channel hit threshold override setting',
    
    -- Pedestals (Ped0 ... Ped15)
    FADC_ped0        FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 0' ,
    FADC_ped1        FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 1', 
    FADC_ped2        FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 2' ,
    FADC_ped3        FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 3' ,
    FADC_ped4        FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 4' ,	
    FADC_ped5        FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 5' ,
    FADC_ped6        FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 6' ,
    FADC_ped7        FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 7' ,
    FADC_ped8        FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 8' ,
    FADC_ped9        FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 9' ,
    FADC_ped10       FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 10', 
    FADC_ped11       FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 11',
    FADC_ped12       FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 12', 
    FADC_ped13       FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 13',	
    FADC_ped14       FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 14', 
    FADC_ped15       FLOAT(10,5) COMMENT 'Pedestal (ADC units) channel 15',

    -- TET Thesholds (TET0 ... TET15)
    FADC_TET0        SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 0', 
    FADC_TET1        SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 1', 
    FADC_TET2        SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 2', 
    FADC_TET3        SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 3', 
    FADC_TET4        SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 4', 
    FADC_TET5        SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 5', 
    FADC_TET6        SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 6', 
    FADC_TET7        SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 7', 
    FADC_TET8        SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 8', 
    FADC_TET9        SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 9', 
    FADC_TET10       SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 10', 
    FADC_TET11       SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 11', 
    FADC_TET12       SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 12', 
    FADC_TET13       SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 13', 
    FADC_TET14       SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 14', 
    FADC_TET15       SMALLINT UNSIGNED COMMENT 'TET threshold (ADC units) channel 15', 

    last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP 
        COMMENT 'Timestamp of last record update',
    
    CONSTRAINT fk_daq_run FOREIGN KEY (run_number) 
        REFERENCES Run_info(run_number)
) ENGINE=InnoDB;

-- 3. EPICS_data Table
CREATE TABLE IF NOT EXISTS EPICS_data (
    -- Primary Run Identifiers
    run_number               INT UNSIGNED PRIMARY KEY,
    epics_run_type           VARCHAR(255) DEFAULT NULL COMMENT 'Run type',
    epics_run_start          DATETIME DEFAULT NULL COMMENT 'Run start timestamp',
    epics_run_end            DATETIME DEFAULT NULL COMMENT 'Run end timestamp',
    epics_run_length         INT UNSIGNED NULL COMMENT 'Run length in seconds',

    -- Accelerator & Beam Energy
    epics_E_beam             FLOAT(10,5) DEFAULT NULL COMMENT 'Beam energy, MeV Hall A [PV: HALLA:p]',
    epics_E_inj              FLOAT(10,5) DEFAULT NULL COMMENT 'Injector energy, MeV [PV: MMSINJEGAIN]',
    epics_E_Slinac           FLOAT(10,5) DEFAULT NULL COMMENT 'South linac energy, MeV [PV: MMSLIN1EGAIN]',
    epics_E_Nlinac           FLOAT(10,5) DEFAULT NULL COMMENT 'North linac energy, MeV [PV: MMSLIN2EGAIN]',
    epics_n_pass             VARCHAR(255) DEFAULT NULL COMMENT 'Passes Hall A [PV: MMSHLAPASS]',

    -- Beam Current Monitors (BCMs) & Unsers
    epics_bcm_avg            FLOAT(10,7) DEFAULT NULL COMMENT 'Beam Current Average [PV: hac_bcm_average]',
    epics_unser              FLOAT(10,7) DEFAULT NULL COMMENT 'Current on Unser monitor [PV: hac_unser_read]',
    epics_bcm_us             FLOAT(10,7) DEFAULT NULL COMMENT 'Current on Upstream bcm [PV: hac_bcm_dvm1_current]',
    epics_bcm_ds             FLOAT(10,7) DEFAULT NULL COMMENT 'Current on Downstream bcm [PV: hac_bcm_dvm2_current]',
    epics_inj_bcm_tot        FLOAT(10,7) DEFAULT NULL COMMENT 'Injector Full Current Monitor 02 [PV: IBC0L02Current]',
    epics_inj_bcm_halla      FLOAT(10,7) DEFAULT NULL COMMENT 'Injector Current Monitor Hall A [PV: IBC1H04CRCUR2]',
    epics_bcm_a1_coeff       FLOAT(10,5) DEFAULT NULL COMMENT 'Upstream Cavity Coefficient [PV: hac_bcm_A1]',
    epics_bcm_a2_coeff       FLOAT(10,5) DEFAULT NULL COMMENT 'Downstream Cavity Coefficient [PV: hac_bcm_A2]',

    -- Beam Position Monitors (BPMs)
    epics_bpm01_X            FLOAT(10,8) DEFAULT NULL COMMENT 'Beam Position BPM01 X, mm [PV: IPM1H01.XPOS]',
    epics_bpm01_Y            FLOAT(10,8) DEFAULT NULL COMMENT 'Beam Position BPM01 Y, mm [PV: IPM1H01.YPOS]',
    epics_bpm04_X            FLOAT(10,8) DEFAULT NULL COMMENT 'Beam Position BPM04 X, mm [PV: IPM1H04.XPOS]',
    epics_bpm04_Y            FLOAT(10,8) DEFAULT NULL COMMENT 'Beam Position BPM04 Y, mm [PV: IPM1H04.YPOS]',
    epics_bpm04a_X           FLOAT(10,8) DEFAULT NULL COMMENT 'Beam Position BPM04A X, mm [PV: IPM1H04A.XPOS]',
    epics_bpm04a_Y           FLOAT(10,8) DEFAULT NULL COMMENT 'Beam Position BPM04A Y, mm [PV: IPM1H04A.YPOS]',
    epics_bpm02a_X           FLOAT(10,8) DEFAULT NULL COMMENT 'Beam Position BPM02A X, mm [PV: IPM1P02A.XPOS]',
    epics_bpm02a_Y           FLOAT(10,8) DEFAULT NULL COMMENT 'Beam Position BPM02A Y, mm [PV: IPM1P02A.YPOS]',

    -- Beamline Magnets
    epics_q1_cur             FLOAT(10,5) DEFAULT NULL COMMENT 'Quad Q1 (Amps) [PV: MQO1H02M]',
    epics_q2_cur             FLOAT(10,5) DEFAULT NULL COMMENT 'Quad Q2 (Amps) [PV: MQM1H02M]',
    epics_q3_cur             FLOAT(10,5) DEFAULT NULL COMMENT 'Quad Q3 (Amps) [PV: MQO1H03M]',
    epics_q4_cur             FLOAT(10,5) DEFAULT NULL COMMENT 'Quad Q4 (Amps) [PV: MQO1H03AM]',
    epics_dip_cur            FLOAT(10,5) DEFAULT NULL COMMENT 'Dipole (Amps) [PV: MMA1H01M]',
    epics_mcz1h0v_cur        FLOAT(10,5) DEFAULT NULL COMMENT 'MCZ1H0V vertical corrector [PV: MBD1H04VM]',

    -- Target System
    epics_target             INT(11) DEFAULT NULL COMMENT 'Target selection ID / state',
    epics_tgt_angle          FLOAT(10,5) DEFAULT NULL COMMENT 'Target Rotary Position(V) [PV: HAHFMROTENC]',
    epics_tgt_rot_neglimit   TINYINT(1) DEFAULT NULL COMMENT 'Rotary Negative Limit Switch [PV: HAHFMROTSM.LLS]',
    epics_tgt_rot_poslimit   TINYINT(1) DEFAULT NULL COMMENT 'Rotary Positive Limit Switch [PV: HAHFMROTSM.HLS]',
    epics_tgt_rot_athome     TINYINT(1) DEFAULT NULL COMMENT 'Rotary Home(Center) Switch [PV: HAHFMROTSM.ATHM]',
    epics_tgt_angle_deg      FLOAT(10,5) DEFAULT NULL COMMENT 'Rotary Position in deg(from controller) [PV: HAHFMROTSM.RBV]',
    epics_tgt_lin_pos        FLOAT(10,5) DEFAULT NULL COMMENT 'Target Linear Position(V) [PV: HAHFMLINENC]',
    epics_tgt_lin_hlimit     TINYINT(1) DEFAULT NULL COMMENT 'Linear Extended Limit Switch [PV: HAHFMLINSM.HLS]',
    epics_tgt_lin_llmit      TINYINT(1) DEFAULT NULL COMMENT 'Linear Retracted Limit Switch [PV: HAHFMLINSM.LLS]',
    epics_tgt_lin_athome     TINYINT(1) DEFAULT NULL COMMENT 'Linear Home switch [PV: HAHFMLINSM.ATHM]',
    epics_tgt_lin_pos_mm     FLOAT(10,5) DEFAULT NULL COMMENT 'Linear Position in mm [PV: HAHFMLINSM.RBV]',

    -- Injector Laser & Slits
    epics_las_mode_halla     VARCHAR(255) DEFAULT NULL COMMENT 'Laser mode Hall A [PV: IGL1I00HALLAMODE]',
    epics_las_mode_hallb     VARCHAR(255) DEFAULT NULL COMMENT 'Laser mode Hall B [PV: IGL1I00HALLBMODE]',
    epics_las_mode_hallc     VARCHAR(255) DEFAULT NULL COMMENT 'Laser mode Hall C [PV: IGL1I00HALLCMODE]',
    epics_las_mode_halld     VARCHAR(255) DEFAULT NULL COMMENT 'Laser mode Hall D [PV: IGL1I00HALLDMODE]',
    epics_las_pow_halla      FLOAT(10,5) DEFAULT NULL COMMENT 'Laser power Hall A [PV: IGL1I00AI3]',
    epics_las_pow_hallb      FLOAT(10,5) DEFAULT NULL COMMENT 'Laser power Hall B [PV: IGL1I00AI4]',
    epics_las_pow_hallc      FLOAT(10,5) DEFAULT NULL COMMENT 'Laser power Hall C [PV: IGL1I00AI5]',
    epics_las_pow_halld      FLOAT(10,5) DEFAULT NULL COMMENT 'Laser power Hall D [PV: IGL1I00AI56',
    epics_las_attn_halla     FLOAT(10,5) DEFAULT NULL COMMENT 'Laser attenuation Hall A [PV: psub_aa_pos]',
    epics_las_attn_hallb     FLOAT(10,5) DEFAULT NULL COMMENT 'Laser attenuation Hall B [PV: psub_ab_pos]',
    epics_las_attn_hallc     FLOAT(10,5) DEFAULT NULL COMMENT 'Laser attenuation Hall C [PV: psub_ac_pos]',
    epics_las_attn_halld     FLOAT(10,5) DEFAULT NULL COMMENT 'Laser attenuation Hall D [PV: psub_ad_pos]',
    epics_slit_halla         FLOAT(10,5) DEFAULT NULL COMMENT 'Slit Position Hall A [PV: SMRPOSA]',
    epics_slit_hallb         FLOAT(10,5) DEFAULT NULL COMMENT 'Slit Position Hall B [PV: SMRPOSB]',
    epics_slit_hallc         FLOAT(10,5) DEFAULT NULL COMMENT 'Slit Position Hall C [PV: SMRPOSC]',
    epics_slit_halld         FLOAT(10,5) DEFAULT NULL COMMENT 'Slit Position Hall D [PV: SMRPOSD]',
    epics_pockels_v1         FLOAT(10,5) DEFAULT NULL COMMENT 'Pockels Cell Voltage 1 [PV: IGL1I00AI7]',
    epics_pockels_v2         FLOAT(10,5) DEFAULT NULL COMMENT 'Pockels Cell Voltage 2 [PV: IGL1I00AI8]',
    epics_las_a_rf_phase     FLOAT(10,5) DEFAULT NULL COMMENT 'Laser A RF phase degrees [PV: R0L1PMES]',
    epics_las_a_src_cur      FLOAT(10,5) DEFAULT NULL COMMENT 'Laser A source current uA [PV: enlk4A:floatspare1]',

    -- Polarization & Wien Filters / Waveplates
    epics_ihwp               VARCHAR(16) DEFAULT NULL COMMENT 'Laser 1/2 wave plate [PV: IGL1I00OD16_16]',
    epics_rhwp               FLOAT(10,5) DEFAULT NULL COMMENT 'Rotating 1/2 wave plate [PV: psub_pl_pos]',
    epics_vwien_bdl          FLOAT(10,5) DEFAULT NULL COMMENT 'VWien BdL [PV: MWF1I04.BDL]',
    epics_vwien_field        FLOAT(10,5) DEFAULT NULL COMMENT 'VWien field [PV: MWF1I04.S]',
    epics_vwien_angle        FLOAT(10,5) DEFAULT NULL COMMENT 'VWien filter angle, deg [PV: VWienAngle]',
    epics_sol_a_bdl          FLOAT(10,5) DEFAULT NULL COMMENT 'Solenoid A BdL [PV: MFG1I04A.BDL]',
    epics_sol_a_field        FLOAT(10,5) DEFAULT NULL COMMENT 'Solenoid A field [PV: MFG1I04A.S]',
    epics_sol_b_bdl          FLOAT(10,5) DEFAULT NULL COMMENT 'Solenoid B BdL [PV: MFG1I04B.BDL]',
    epics_sol_b_field        FLOAT(10,5) DEFAULT NULL COMMENT 'Solenoid B field [PV: MFG1I04B.S]',
    epics_sol_phi_fg         FLOAT(10,5) DEFAULT NULL COMMENT 'Solenoids angle, deg [PV: Phi_FG]',
    epics_hwien_bdl          FLOAT(10,5) DEFAULT NULL COMMENT 'HWien BdL [PV: MWF1I06.BDL]',
    epics_hwien_field        FLOAT(10,5) DEFAULT NULL COMMENT 'HWien field [PV: MWF1I06.S]',
    epics_hwien_angle        FLOAT(10,5) DEFAULT NULL COMMENT 'HWien filter angle, deg [PV: HWienAngle]',

    -- Helicity Board Settings
    epics_hel_pattern        VARCHAR(255) DEFAULT NULL COMMENT 'Helicity pattern pair, quartet, octet... [PV: HELPATTERNd]',
    epics_hel_freq           DECIMAL(12,8) DEFAULT NULL COMMENT 'Helicity flip frequency [PV: HELFREQ]',
    epics_hel_delay          VARCHAR(255) DEFAULT NULL COMMENT 'Helicity delay in units of windows [PV: HELDELAYd]',
    epics_t_settle           FLOAT(10,5) DEFAULT NULL COMMENT 'Tsettle window MPS signal, usec [PV: HELTSETTLEd]',
    epics_t_stable           FLOAT(10,5) DEFAULT NULL COMMENT 'Helicity Tstable window, usec [PV: HELTSTABLEd]',

    -- Superconducting Solenoid Magnet (Møller Polarimeter)
    epics_mol_pow_sup_cur    	FLOAT(10,5) DEFAULT NULL COMMENT 'AM430 Current Setpoint (A) [PV: hamolpol:am430:target]',
    epics_mol_mag_cur_set    	FLOAT(10,5) DEFAULT NULL COMMENT 'AM430 Current Setpoint Readback (A) [PV: hamolpol:am430:targetRbck]',
    epics_mol_mag_htr_ctrl   	FLOAT(10,5) DEFAULT NULL COMMENT 'AM430 Persistence Heater Control [PV: hamolpol:am430:turnOn]',
    epics_mol_mag_cur_meas   	FLOAT(10,5) DEFAULT NULL COMMENT 'AM430 Measured Current (A) [PV: hamolpol:am430:magCurrent]',
    epics_mol_mag_v_meas     	FLOAT(10,5) DEFAULT NULL COMMENT 'AM430 Measured Voltage (A) [PV: hamolpol:am430:magVoltage]',
    epics_mol_mag_field_meas 	FLOAT(10,5) DEFAULT NULL COMMENT 'AM430 Measured Field (T) [PV: hamolpol:am430:magField]',
    epics_mol_mag_ramp_state 	FLOAT(10,5) DEFAULT NULL COMMENT 'AMS430 Ramp State [PV: hamolpol:am430:rampState]',
    epics_mol_cooler_temp    	FLOAT(10,5) DEFAULT NULL COMMENT 'Cryocooler Temperature (K) [PV: hamolpol:lk218_1:temp1]',
    epics_mol_mag_T2temp     	FLOAT(10,5) DEFAULT NULL COMMENT 'Magnet(T2) Temperature(K) [PV: hamolpol:lk218_1:temp2]',
    epics_mol_mag_lead1_temp 	FLOAT(10,5) DEFAULT NULL COMMENT 'Magnet Lead #1 (T6) Temperature(K) [PV: hamolpol:lk218_1:temp6]',
    epics_mol_mag_lead2_temp 	FLOAT(10,5) DEFAULT NULL COMMENT 'Magnet Lead #2 (T7) Temperature(K) [PV: hamolpol:lk218_1:temp7]',
    epics_mol_ext_gaussmeter 	FLOAT(10,5) DEFAULT NULL COMMENT 'Magnet External Gaussmeter (kGs) [PV: hamolpol:lk450:fld]',

    -- Cryomech Compressor System
    epics_cryo_comp_op_time	FLOAT(10,5) DEFAULT NULL COMMENT 'Cryomech CP1110 Compressor Oper.Time (min) [PV: hamolpol:CP2800:compMins]',
    epics_cryo_water_in_temp 	FLOAT(10,5) DEFAULT NULL COMMENT 'Cryomech CP1110 Input Water Temp (C) [PV: hamolpol:CP2800:waterInTemp]',
    epics_cryo_water_out_temp 	FLOAT(10,5) DEFAULT NULL COMMENT 'Cryomech CP1110 Output Water Temp (C) [PV: hamolpol:CP2800:waterOutTemp]',
    epics_cryo_press_high    	FLOAT(10,5) DEFAULT NULL COMMENT 'Cryomech CP1110 High Side Pressure (PSI) [PV: hamolpol:CP2800:pressHigh]',
    epics_cryo_press_low     	FLOAT(10,5) DEFAULT NULL COMMENT 'Cryomech CP1110 Low Side Pressure (PSI) [PV: hamolpol:CP2800:pressLow]',

    -- Detector High Voltage Readbacks (Channels 1 - 8)
    epics_det_hv_ch1         FLOAT(10,5) DEFAULT NULL COMMENT 'HA Moller HV Readback Ch 1 (V) [PV: IHVHAPOL:03:000:VMon]',
    epics_det_hv_ch2         FLOAT(10,5) DEFAULT NULL COMMENT 'HA Moller HV Readback Ch 2 (V) [PV: IHVHAPOL:03:001:VMon]',
    epics_det_hv_ch3         FLOAT(10,5) DEFAULT NULL COMMENT 'HA Moller HV Readback Ch 3 (V) [PV: IHVHAPOL:03:002:VMon]',
    epics_det_hv_ch4         FLOAT(10,5) DEFAULT NULL COMMENT 'HA Moller HV Readback Ch 4 (V) [PV: IHVHAPOL:03:003:VMon]',
    epics_det_hv_ch5         FLOAT(10,5) DEFAULT NULL COMMENT 'HA Moller HV Readback Ch 5 (V) [PV: IHVHAPOL:03:004:VMon]',
    epics_det_hv_ch6         FLOAT(10,5) DEFAULT NULL COMMENT 'HA Moller HV Readback Ch 6 (V) [PV: IHVHAPOL:03:005:VMon]',
    epics_det_hv_ch7         FLOAT(10,5) DEFAULT NULL COMMENT 'HA Moller HV Readback Ch 7 (V) [PV: IHVHAPOL:03:006:VMon]',
    epics_det_hv_ch8         FLOAT(10,5) DEFAULT NULL COMMENT 'HA Moller HV Readback Ch 8 (V) [PV: IHVHAPOL:03:007:VMon]',
    last_updated 	     TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'Timestamp of last record update',

    CONSTRAINT fk_epics_run FOREIGN KEY (run_number) 
        REFERENCES Run_info(run_number)
) ENGINE=InnoDB;

-- 4. Analysis Table
CREATE TABLE IF NOT EXISTS Analysis (
    run_number 	     INT UNSIGNED PRIMARY KEY,

    -- Event Rates & Scalers
    leftrate 	     FLOAT DEFAULT NULL COMMENT 'Left detector rate (Hz)',
    leftrate_err     FLOAT DEFAULT NULL COMMENT 'Error on left detector rate (Hz)',
    rightrate 	     FLOAT DEFAULT NULL COMMENT 'Right detector rate (Hz)',
    rightrate_err    FLOAT DEFAULT NULL COMMENT 'Error on right detector rate (Hz)',
    coinrate 	     FLOAT DEFAULT NULL COMMENT 'Coincidence rate (Hz)',
    coinrate_err     FLOAT DEFAULT NULL COMMENT 'Error on coincidence rate (Hz)',
    accrate 	     FLOAT DEFAULT NULL COMMENT 'Accidental coincidence rate (Hz)',
    accrate_err      FLOAT DEFAULT NULL COMMENT 'Error on accidental coincidence rate (Hz)',
    counts_to_Hz     FLOAT DEFAULT NULL COMMENT 'Multiplicative factor to get from counts per window to rate. Should be 1/T_settle.',	     

    -- Diagnostics & Timing
    bcm			INT DEFAULT NULL COMMENT 'Beam current monitor scaler counts',
    clock100kHz      	INT  DEFAULT NULL COMMENT '100kHz Clock scaler ticks',
    clock20MHz       	INT  DEFAULT NULL COMMENT '20MHz Clock scaler ticks',
    deadtime_tau_1 	DOUBLE COMMENT 'Dead time constant (s) for coinc-coinc pile up',
    deadtime_tau_2 	DOUBLE COMMENT 'Dead time constant (s) for single-coinc pile up',
    accid_tau 	   	DOUBLE COMMENT 'Accidental window width (s)',	       


    -- Asymmetries & Polarization Parameters
    asym_mol 	     	FLOAT DEFAULT NULL COMMENT 'Measured raw asymmetry',
    asym_mol_err     	FLOAT DEFAULT NULL COMMENT 'Statistical error on raw asymmetry',
    Azz 		FLOAT(10,8) DEFAULT NULL COMMENT 'Analyzing power Azz',
    poltarg 	     	FLOAT(10,8) DEFAULT NULL COMMENT 'Target polarization',
    pol_beam 	     	FLOAT DEFAULT NULL COMMENT 'Extracted beam polarization',
    pol_beam_err     	FLOAT DEFAULT NULL COMMENT 'Error on beam polarization',

    -- Charge Asymmetry & Pedestals
    asym_q    		FLOAT DEFAULT NULL COMMENT 'Charge asymmetry',
    asym_q_err 		FLOAT DEFAULT NULL COMMENT 'Error on charge asymmetry',
    qpedused 		FLOAT DEFAULT NULL COMMENT 'BCM pedestal value used in analysis',
    qpedcalc 		FLOAT(7,6) DEFAULT NULL COMMENT 'Calculated BCM pedestal value',

    last_updated 	TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP 
        		COMMENT 'Timestamp of last record update',
    
    CONSTRAINT fk_analysis_run FOREIGN KEY (run_number) 
        REFERENCES Run_info(run_number)
) ENGINE=InnoDB;

EOF
)

# --- Execution ---
echo "Connecting to hamoller_db via alias 'hamoller_sql_user'..."

# Execute SQL script using your alias
hamoller_sql_admin <<< "$SQL_SCRIPT"

echo "Database tables (Run_info, DAQ_config, EPICS_data, Analysis) successfully created!"
