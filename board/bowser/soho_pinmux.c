/* SOHO pinmux*/
/* Modified pin muxs */
#define MUX_SOHO() \
	MV(CP(GPMC_AD0) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat0 */ \
	MV(CP(GPMC_AD1) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat1 */ \
	MV(CP(GPMC_AD2) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat2 */ \
	MV(CP(GPMC_AD3) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat3 */ \
	MV(CP(GPMC_AD4) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat4 */ \
	MV(CP(GPMC_AD5) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat5 */ \
	MV(CP(GPMC_AD6) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat6 */ \
	MV(CP(GPMC_AD7) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_dat7 */ \
	MV(CP(GPMC_AD8) , ( PTU | IEN | M3))  /* gpio_32 => emmc_reset_b_1v8 */ \
	MV(CP(GPMC_AD9) , ( PTU | IEN | M3))  /* gpio_33 => emmc_wp_sw */ \
	MV(CP(GPMC_AD10) , ( PTU | IEN | M3))  /* gpio_34 => emmc_cd_sw */ \
	MV(CP(GPMC_AD11) , ( PTD | IEN | M3))  /* gpio_35 LCDPANEL_EN */ \
	MV(CP(GPMC_AD13) , ( PTD | IEN | M3))  /* gpio_37 */ \
	MV(CP(GPMC_A17) , ( PTD | M3))  /* gpio_41 LED_TST_GPIO41 */ \
	MV(CP(GPMC_A19) , ( PTD | IEN |  M3))  /* gpio_43 => WLAN_EN */ \
	MV(CP(GPMC_A20) , ( PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M3))  /* gpio_44 => WLAN_WAKE */ \
	MV(CP(GPMC_A21) , ( PTU | IEN | M3))  /* gpio_45 => CHRG_SUSP */ \
	MV(CP(GPMC_A22) , ( PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M3))  /* gpio_46 => BT_EN */ \
	MV(CP(GPMC_A23) , ( PTU | IEN | M3))  /* gpio_47 => CHRG_USB_HCS */ \
	MV(CP(GPMC_A25) , ( PTD | M3))  /* gpio_49 => BT_DEV_WAKE */ \
	MV(CP(GPMC_NCS0) , ( PTU | IEN | M3))  /* gpio_50 => volume_down */ \
	MV(CP(GPMC_NCS3) , ( PTD | M3))  /* gpio_53 => BT_RST_N */ \
	MV(CP(GPMC_NWP) , ( PTD | IEN | M3))  /* gpio_54 => GPADC_START */ \
	MV(CP(GPMC_NOE) , ( PTU | IEN | OFF_EN | OFF_OUT_PTD | M1))  /* sdmmc2_clk */ \
	MV(CP(GPMC_NWE) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* sdmmc2_cmd */ \
	MV(CP(C2C_DATA14) , ( PTU | IEN | M3))  /* gpio_103 board_id (eng vs prod) */ \
	MV(CP(C2C_DATA15) , ( PTD | IEN | M3))  /* gpio_104 USB_OTG_JACK_ID */ \
	MV(CP(CAM_STROBE),		(PTD | IDIS | M3)) /* gpio_82 BT_GPIO1_WAKE */ \
	MV(CP(USBB1_ULPITLL_DAT3) , ( PTD | IEN | M3))  /* gpio_91 TST_BD_ID3 */ \
	MV(CP(ABE_MCBSP2_CLKX) , ( IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* h_mcbsp2_clkx */ \
	MV(CP(ABE_MCBSP2_DR) , ( IEN | OFF_EN | OFF_OUT_PTD | M0))  /* h_mcbsp2_dr */ \
	MV(CP(ABE_MCBSP2_DX) , ( OFF_EN | OFF_OUT_PTD | M0))  /* h_mcbsp2_dx */ \
	MV(CP(ABE_MCBSP2_FSX) , ( IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* h_mcbsp2_fsx */ \
	MV(CP(ABE_PDM_UL_DATA) , ( PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* h_mcbsp3_dr */ \
	MV(CP(ABE_PDM_DL_DATA) , ( PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* h_mcbsp3_dx */ \
	MV(CP(ABE_PDM_FRAME) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* h_mcbsp3_fsx */ \
	MV(CP(ABE_PDM_LB_CLK) , ( PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* h_mcbsp3_clk */ \
	MV(CP(UART2_CTS) , ( PTU | IEN | M0))  /* uart2_cts  hci_cts */ \
	MV(CP(UART2_RTS) , ( M0))  /* uart2_rts hci_rts */ \
	MV(CP(UART2_RX) , ( PTU | IEN | M0))  /* uart2_rx hci_rx */ \
	MV(CP(UART2_TX) , ( M0))  /* uart2_tx hci_tx */ \
	MV(CP(I2C1_SCL) , ( PTU | IEN | M0))  /* i2c1_scl */ \
	MV(CP(I2C1_SDA) , ( PTU | IEN | M0))  /* i2c1_sda */ \
	MV(CP(I2C2_SCL) , ( PTU | IEN | M0))  /* i2c2_scl */ \
	MV(CP(I2C2_SDA) , ( PTU | IEN | M0))  /* i2c2_sda */ \
	MV(CP(I2C3_SCL) , ( PTU | IEN | M0))  /* i2c3_scl */ \
	MV(CP(I2C3_SDA) , ( PTU | IEN | M0))  /* i2c3_sda */ \
	MV(CP(I2C4_SCL) , ( PTU | IEN | M0))  /* i2c4_scl */ \
	MV(CP(I2C4_SDA) , ( PTU | IEN | M0))  /* i2c4_sda */ \
	MV(CONTROL_I2C_0,     (0x2422) ) /* i2c_0 : I2C3 SDA PU strength=860E */ \
	MV((CONTROL_I2C_0+2), (0x2422) ) /* i2c_0 : I2C3 SCL PU strength=860E */ \
	MV(CP(UART3_RX_IRRX) , ( IEN | M0))  /* uart3_rx - debug*/ \
	MV(CP(UART3_TX_IRTX) , ( M0))  /* uart3_tx - debug */ \
	MV(CP(SDMMC5_CLK) , ( PTU | IEN | OFF_EN | OFF_OUT_PTD | M0))  /* sdmmc5_clk wlan_sdio_clk */ \
	MV(CP(SDMMC5_CMD) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* sdmmc5_cmd wlan_sdio_cmd */ \
	MV(CP(SDMMC5_DAT0) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* sdmmc5_dat0 wlan_sdio_dat0 */ \
	MV(CP(SDMMC5_DAT1) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* sdmmc5_dat1 wlan_sdio_dat1 */ \
	MV(CP(SDMMC5_DAT2) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* sdmmc5_dat2 wlan_sdio_dat2 */ \
	MV(CP(SDMMC5_DAT3) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* sdmmc5_dat3 wlan_sdio_dat3 */ \
	MV(CP(UNIPRO_TY0) , ( PTD | IEN | M3))  /* gpio_172 TST_BD_ID1 */ \
	MV(CP(UNIPRO_TX0), (PTU | IEN  | M3)) /* HDSET_DETECT */ \
	MV(CP(UNIPRO_TX1), (PTU | IEN  | M3)) /* CHRG_SYS_OK */ \
	MV(CP(UNIPRO_TY1) , ( PTD | IEN | M3))  /* gpio_174 TST_BD_ID2 */ \
	MV(CP(USBA0_OTG_DP) , ( IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* usba0_otg_dp */ \
	MV(CP(USBA0_OTG_DM) , ( IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* usba0_otg_dm */ \
	MV(CP(FREF_CLK2_OUT) , ( M0))  /* fref_clk2_out */ \
	MV(CP(DPM_EMU12) , ( PTD | M3))  /* gpio_23 - TOUCH_RST_N */ \
	MV(CP(DPM_EMU13) , ( PTU | IEN | M3))  /* gpio_24 TOUCH_INT_N */ \
	MV(CP(DPM_EMU14) , ( M3))  /* gpio_25 - LCDBKL_EN */ \
	MV(CP(DPM_EMU15) , ( IEN | M3))  /* gpio_26 AUDIO_GPIO1_A */ \
	/* MV(CP(DPM_EMU18) , ( M1))  gpio_190 - LCDBKL_PWM (CABC enable) */ \
	MV1(WK(PAD0_SIM_IO) , ( PTU | IEN | M3))  /* gpio_wk0 HALL_SEN_INT_N */ \
	MV1(WK(PAD1_SIM_CLK) , ( PTU | IEN | M3))  /* gpio_wk1 volume_up */ \
	MV1(WK(PAD0_SIM_RESET) , ( IEN | M3))  /* gpio_wk2 chrg_stat */ \
	MV1(WK(PAD1_SIM_CD) , ( PTD | M3))  /* gpio_wk3 LCDPANEL_1V8_EN */ \
	MV1(WK(PAD0_SIM_PWRCTRL) , ( PTD | IEN | M3))  /* gpio_wk4 AIRQ1_INT */ \
	MV1(WK(PAD1_SR_SCL) , ( PTU | IEN | M0))  /* sr_scl */ \
	MV1(WK(PAD0_SR_SDA) , ( PTU | IEN | M0))  /* sr_sda */ \
	MV1(WK(PAD1_FREF_XTAL_IN) , ( M0))  /* fref_xtal_in */ \
	MV1(WK(PAD0_FREF_SLICER_IN) , ( M0))  /* fref_slicer_in */ \
	MV1(WK(PAD1_FREF_CLK_IOREQ) , ( PTD | M0))  /* fref_clk_ioreq */ \
	MV1(WK(PAD0_FREF_CLK0_OUT) , ( M0))  /* fref_clk0_out Audio*/ \
	MV1(WK(PAD1_FREF_CLK3_REQ) , ( PTD | IEN | M2))  /* sys_drm_msecure */ \
	MV1(WK(PAD1_FREF_CLK4_REQ),	(IEN | M3))  /* gpio_wk7 ON_BUTTON */ \
