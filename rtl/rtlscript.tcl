# This is a generated script based on design: mtxdev
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
#
# To test this script, run the following commands from Vivado Tcl console:
# source mtxdev_script.tcl

# If you do not already have a project created,
# you can create a project using the following command:
#    create_project project_1 myproj -part xc7vx485tffg1157-1 -force

# If you do not already have an existing IP Integrator design open,
# you can create a design using the following command:
#    create_bd_design mtxdev

# Creating design if needed
if { [get_files *.bd] eq "" } {
   puts "INFO: Currently there are no designs in project, so creating one..."
   create_bd_design mtxdev
}



# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     puts "ERROR: Unable to find parent cell <$parentCell>!"
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     puts "ERROR: Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports
  set DDR [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:ddrx_rtl:1.0 DDR ]
  set FIXED_IO [ create_bd_intf_port -mode Master -vlnv xilinx.com:display_processing_system7:fixedio_rtl:1.0 FIXED_IO ]

  # Create ports

  # Create instance: processing_system7_1, and set properties
  set processing_system7_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.2 processing_system7_1 ]
  set_property -dict [ list CONFIG.PCW_USE_M_AXI_GP1 {0} CONFIG.PCW_USE_S_AXI_GP0 {1} CONFIG.PCW_USE_S_AXI_HP0 {0} CONFIG.PCW_USE_FABRIC_INTERRUPT {1} CONFIG.PCW_S_AXI_HP0_DATA_WIDTH {32} CONFIG.PCW_IRQ_F2P_INTR {1}  ] $processing_system7_1

  # Create instance: processing_system7_1_axi_periph, and set properties
  set processing_system7_1_axi_periph [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.0 processing_system7_1_axi_periph ]
  set_property -dict [ list CONFIG.NUM_SI {2} CONFIG.NUM_MI {2}  ] $processing_system7_1_axi_periph

  # Create instance: proc_sys_reset, and set properties
  set proc_sys_reset [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 proc_sys_reset ]

  # Create instance: blk_mem_gen_1, and set properties
  set blk_mem_gen_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:8.0 blk_mem_gen_1 ]
  set_property -dict [ list CONFIG.Memory_Type {True_Dual_Port_RAM} CONFIG.use_bram_block {BRAM_Controller}  ] $blk_mem_gen_1

  # Create instance: dev_hls_1, and set properties
  set dev_hls_1 [ create_bd_cell -type ip -vlnv xilinx.com:hls:dev_hls:1.0 dev_hls_1 ]
  set_property -dict [ list CONFIG.C_M_AXI_AXIBUS_ID_WIDTH {1}  ] $dev_hls_1

  # Create interface connections
  connect_bd_intf_net -intf_net processing_system7_1_ddr [get_bd_intf_ports DDR] [get_bd_intf_pins processing_system7_1/DDR]
  connect_bd_intf_net -intf_net processing_system7_1_fixed_io [get_bd_intf_ports FIXED_IO] [get_bd_intf_pins processing_system7_1/FIXED_IO]
  connect_bd_intf_net -intf_net processing_system7_1_m_axi_gp0 [get_bd_intf_pins processing_system7_1/M_AXI_GP0] [get_bd_intf_pins processing_system7_1_axi_periph/S00_AXI]
  connect_bd_intf_net -intf_net processing_system7_1_axi_periph_m00_axi [get_bd_intf_pins processing_system7_1_axi_periph/M00_AXI] [get_bd_intf_pins dev_hls_1/S_AXI_SLAVE]
  connect_bd_intf_net -intf_net dev_hls_1_imem_porta [get_bd_intf_pins dev_hls_1/imem_PORTA] [get_bd_intf_pins blk_mem_gen_1/BRAM_PORTA]
  connect_bd_intf_net -intf_net dev_hls_1_imem_portb [get_bd_intf_pins dev_hls_1/imem_PORTB] [get_bd_intf_pins blk_mem_gen_1/BRAM_PORTB]
  connect_bd_intf_net -intf_net processing_system7_1_axi_periph_m01_axi [get_bd_intf_pins processing_system7_1_axi_periph/M01_AXI] [get_bd_intf_pins processing_system7_1/S_AXI_GP0]
  connect_bd_intf_net -intf_net dev_hls_1_m_axi_axibus [get_bd_intf_pins dev_hls_1/M_AXI_AXIBUS] [get_bd_intf_pins processing_system7_1_axi_periph/S01_AXI]

  # Create port connections
  connect_bd_net -net processing_system7_1_fclk_clk0 [get_bd_pins processing_system7_1/FCLK_CLK0] [get_bd_pins processing_system7_1/M_AXI_GP0_ACLK] [get_bd_pins processing_system7_1_axi_periph/S00_ACLK] [get_bd_pins proc_sys_reset/slowest_sync_clk] [get_bd_pins processing_system7_1_axi_periph/ACLK] [get_bd_pins processing_system7_1_axi_periph/M00_ACLK] [get_bd_pins dev_hls_1/aclk] [get_bd_pins processing_system7_1/S_AXI_GP0_ACLK] [get_bd_pins processing_system7_1_axi_periph/S01_ACLK] [get_bd_pins processing_system7_1_axi_periph/M01_ACLK]
  connect_bd_net -net processing_system7_1_fclk_reset0_n [get_bd_pins processing_system7_1/FCLK_RESET0_N] [get_bd_pins proc_sys_reset/ext_reset_in]
  connect_bd_net -net proc_sys_reset_peripheral_aresetn [get_bd_pins proc_sys_reset/peripheral_aresetn] [get_bd_pins processing_system7_1_axi_periph/S00_ARESETN] [get_bd_pins processing_system7_1_axi_periph/M00_ARESETN] [get_bd_pins dev_hls_1/aresetn] [get_bd_pins processing_system7_1_axi_periph/S01_ARESETN] [get_bd_pins processing_system7_1_axi_periph/M01_ARESETN]
  connect_bd_net -net proc_sys_reset_interconnect_aresetn [get_bd_pins proc_sys_reset/interconnect_aresetn] [get_bd_pins processing_system7_1_axi_periph/ARESETN]
  connect_bd_net -net dev_hls_1_interrupt [get_bd_pins dev_hls_1/interrupt] [get_bd_pins processing_system7_1/IRQ_F2P]

  # Create address segments
  create_bd_addr_seg -range 0x10000 -offset 0x43C00000 [get_bd_addr_spaces processing_system7_1/Data] [get_bd_addr_segs dev_hls_1/S_AXI_SLAVE/Reg] SEG1
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces dev_hls_1/Data] [get_bd_addr_segs processing_system7_1/S_AXI_GP0/GP0_DDR_LOWOCM] SEG1
  create_bd_addr_seg -range 0x1000000 -offset 0xFC000000 [get_bd_addr_spaces dev_hls_1/Data] [get_bd_addr_segs processing_system7_1/S_AXI_GP0/GP0_QSPI_LINEAR] SEG2
  create_bd_addr_seg -range 0x400000 -offset 0xE0000000 [get_bd_addr_spaces dev_hls_1/Data] [get_bd_addr_segs processing_system7_1/S_AXI_GP0/GP0_IOP] SEG3
  

  # Restore current instance
  current_bd_instance $oldCurInst
}


#####################################################
# Main Flow
#####################################################

create_root_design ""
