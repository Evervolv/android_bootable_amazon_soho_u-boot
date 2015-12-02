/* tate pinmux for SCO */
#define MUX_SECONDARY_TATE() \
	MV(CP(ABE_MCBSP2_CLKX) , ( IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* h_mcbsp2_clkx */ \
        MV(CP(ABE_MCBSP2_DR) , ( IEN | OFF_EN | OFF_OUT_PTD | M0))  /* h_mcbsp2_dr */ \
        MV(CP(ABE_MCBSP2_DX) , ( OFF_EN | OFF_OUT_PTD | M0))  /* h_mcbsp2_dx */ \
        MV(CP(ABE_MCBSP2_FSX) , ( IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* h_mcbsp2_fsx */ \
        MV(CP(ABE_MCBSP1_CLKX) , ( IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* abe_mcbsp1_clkx */ \
        MV(CP(ABE_MCBSP1_DR) , ( IEN | OFF_EN | OFF_OUT_PTD | M0))  /* abe_mcbsp1_dr */ \
        MV(CP(ABE_MCBSP1_DX) , ( OFF_EN | OFF_OUT_PTD | M0))  /* abe_mcbsp1_dx */ \
        MV(CP(ABE_MCBSP1_FSX) , ( IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* abe_mcbsp1_fsx */ \
        MV(CP(ABE_PDM_UL_DATA) , ( PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* abe_mcbsp3_dr */ \
        MV(CP(ABE_PDM_DL_DATA) , ( PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* abe_mcbsp3_dx */ \
        MV(CP(ABE_PDM_FRAME) , ( PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* abe_mcbsp3_fsx */ \
        MV(CP(ABE_PDM_LB_CLK) , ( PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M1))  /* abe_mcbsp3_clk */ \
        MV(CP(ABE_CLKS) , ( PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0))  /* abe_clks */ \
        MV(CP(ABE_DMIC_CLK1) , ( M0))  /* abe_dmic_clk1 */ \
        MV(CP(ABE_DMIC_DIN1) , ( IEN | M0))  /* abe_dmic_din1 */ \
        MV(CP(ABE_DMIC_DIN2) , ( PTD | IEN | M3))  /* gpio_121 */ \
        MV(CP(ABE_DMIC_DIN3) , ( PTU | IEN | M3))  /* gpio_122 */

