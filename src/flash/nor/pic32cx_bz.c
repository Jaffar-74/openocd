// // SPDX-License-Identifier: GPL-2.0-or-later

// /**************************************************************************************
//  *   Copyright (C) 2025 by Microchip Technologies Inc                                  *
//  *   Author: Dinesh Arasu - dinesh.arasu@microchip.com                                 *
//  *                                                                                     *
//  *   Description: Flash driver for WBZ and PIC32CX_BZx Microchip Curiosity Board        *
//  **************************************************************************************/

// #ifdef HAVE_CONFIG_H
// #include "config.h"
// #endif

// #include "imp.h"
// #include "helper/binarybuffer.h"
// #include <helper/time_support.h>
// #include <jtag/jtag.h>
// #include <target/cortex_m.h>
// #include <stdbool.h>
// #include <string.h>
// #include <target/target_type.h>
// #include <stdlib.h>
// #include <inttypes.h>

// #define DSU_DID_REG 0x41000018U

// /* Generic NVM operation bits */
// #define NVMCON_NVMWREN          (1 << 14)
// #define NVMCON_NVMWR            (1 << 15)
// #define NVMCON_OP_WORD_PROG     0x4001
// #define NVMCON_OP_ROW_PROG      0x4003
// #define NVMCON_OP_PAGE_ERASE    0x4004
// #define NVMCON_OP_PBC           0x4007
// #define NVMCON_OP_MASK          0x7FFF

// /* RAM staging buffer address used across drivers */
// #define RAM_BUF_ADDR 0x20000000U

// /* default row size used by these parts */
// #define DEFAULT_ROW_SIZE 1024U

// /* NVMLBWP unlock key used in originals */
// #define NVMLBWP_UNLOCK_KEY 0x80000000U

// struct pic32cx_dev {
//     uint32_t device_id;
//     const char *name;
//     uint32_t flash_base;
//     uint32_t flash_size;
//     uint32_t row_size;
//     uint32_t page_size;
//     uint32_t pac_base;
//     uint32_t nvmcon;
//     uint32_t nvmkey;
//     uint32_t nvmaddr;
//     uint32_t nvmsrcaddr;
//     uint32_t nvmdata;
//     uint32_t nvmconset;
//     uint32_t nvmconclr;
//     uint32_t nvmlbwp;
//     uint32_t nvm_param;
//     uint32_t nvm_err_mask;
// };

// static const struct pic32cx_dev device_table[] = {
//     {
//         .device_id    = 0x00009B8F, /* WBZ451 */
//         .name         = "WBZ451",
//         .flash_base   = 0x01000000U,
//         .flash_size   = 0x00100000U, /* 1 MB */
//         .row_size     = DEFAULT_ROW_SIZE,
//         .page_size    = DEFAULT_ROW_SIZE,
//         .pac_base     = 0x40000000U,
//         .nvmcon       = 0x44000600U,
//         .nvmkey       = 0x44000620U,
//         .nvmaddr      = 0x44000630U,
//         .nvmsrcaddr   = 0x440006C0U,
//         .nvmdata      = 0x44000640U,
//         .nvmconset    = 0x44000608U,
//         .nvmconclr    = 0x44000604U,
//         .nvmlbwp      = 0x440006F0U,
//         .nvm_param    = 0x44000610U,
//         .nvm_err_mask = 0x3F00U
//     },
//     {
//         .device_id    = 0x00009B0B, /* WBZ450 */
//         .name         = "WBZ450",
//         .flash_base   = 0x01000000U,
//         .flash_size   = 0x00100000U, /* 1 MB */
//         .row_size     = DEFAULT_ROW_SIZE,
//         .page_size    = DEFAULT_ROW_SIZE,
//         .pac_base     = 0x40000000U,
//         .nvmcon       = 0x44000600U,
//         .nvmkey       = 0x44000620U,
//         .nvmaddr      = 0x44000630U,
//         .nvmsrcaddr   = 0x440006C0U,
//         .nvmdata      = 0x44000640U,
//         .nvmconset    = 0x44000608U,
//         .nvmconclr    = 0x44000604U,
//         .nvmlbwp      = 0x440006F0U,
//         .nvm_param    = 0x44000610U,
//         .nvm_err_mask = 0x3F00U
//     },
//     {
//         .device_id    = 0x00009B8F, /* WBZ451HPE */
//         .name         = "WBZ451H",
//         .flash_base   = 0x01000000U,
//         .flash_size   = 0x00100000U, /* 1 MB */
//         .row_size     = DEFAULT_ROW_SIZE,
//         .page_size    = DEFAULT_ROW_SIZE,
//         .pac_base     = 0x40000000U,
//         .nvmcon       = 0x44000600U,
//         .nvmkey       = 0x44000620U,
//         .nvmaddr      = 0x44000630U,
//         .nvmsrcaddr   = 0x440006C0U,
//         .nvmdata      = 0x44000640U,
//         .nvmconset    = 0x44000608U,
//         .nvmconclr    = 0x44000604U,
//         .nvmlbwp      = 0x440006F0U,
//         .nvm_param    = 0x44000610U,
//         .nvm_err_mask = 0x3F00U
//     },
//     {
//         .device_id    = 0x00009E03, /* WBZ351 */
//         .name         = "WBZ351",
//         .flash_base   = 0x01000000U,
//         .flash_size   = 0x00080000U, /* 512 KB */
//         .row_size     = DEFAULT_ROW_SIZE,
//         .page_size    = DEFAULT_ROW_SIZE,
//         .pac_base     = 0x40000000U,
//         .nvmcon       = 0x44000600U,
//         .nvmkey       = 0x44000620U,
//         .nvmaddr      = 0x44000630U,
//         .nvmsrcaddr   = 0x440006C0U,
//         .nvmdata      = 0x44000640U,
//         .nvmconset    = 0x44000608U,
//         .nvmconclr    = 0x44000604U,
//         .nvmlbwp      = 0x440006F0U,
//         .nvm_param    = 0x44000610U,
//         .nvm_err_mask = 0x3F00U
//     },
//     {
//         .device_id    = 0x0001A800, /* PIC32WM_BZ6204 */
//         .name         = "PIC32WM",
//         .flash_base   = 0x01000000U,
//         .flash_size   = 0x00200000U, /* 2 MB */
//         .row_size     = DEFAULT_ROW_SIZE,
//         .page_size    = DEFAULT_ROW_SIZE,
//         .pac_base     = 0x40000000U,
//         .nvmcon       = 0x44000600U,
//         .nvmkey       = 0x44000620U,
//         .nvmaddr      = 0x44000630U,
//         .nvmsrcaddr   = 0x440006C0U,
//         .nvmdata      = 0x44000640U,
//         .nvmconset    = 0x44000608U,
//         .nvmconclr    = 0x44000604U,
//         .nvmlbwp      = 0x440006F0U,
//         .nvm_param    = 0x44000610U,
//         .nvm_err_mask = 0x3F00U
//     }
// };

// struct pic32cx_priv {
//     struct target *target;
//     bool probed;
//     const struct pic32cx_dev *dev;
//     uint32_t page_size;
//     uint32_t num_pages;
// };

// static const struct pic32cx_dev *find_device_by_did(uint32_t did)
// {
//     for (size_t i = 0; i < ARRAY_SIZE(device_table); ++i)
//         if (device_table[i].device_id != 0 && device_table[i].device_id == did)
//             return &device_table[i];
//     return NULL;
// }

// static const struct pic32cx_dev *find_device_by_name(const char *name)
// {
//     if (!name) return NULL;
//     for (size_t i = 0; i < ARRAY_SIZE(device_table); ++i)
//         if (strstr(name, device_table[i].name) != NULL)
//             return &device_table[i];
//     return NULL;
// }

// /* unlock via PAC if locked (same as originals) */
// static int pic32cx_unlock_flash(const struct pic32cx_dev *dev, struct target *t)
// {
//     uint32_t status;
//     int res = target_read_u32(t, dev->pac_base + 0x18U, &status);
//     if (res != ERROR_OK) return res;

//     if (status & (1 << 1)) {
//         LOG_INFO("PAC indicates NVMCTRL is locked. Attempting to unlock...");
//         res = target_write_u32(t, dev->pac_base + 0x20U, (1 << 1));
//         if (res != ERROR_OK) return res;
//     }
//     return target_read_u32(t, dev->pac_base + 0x18U, &status);
// }

// /* Generic NVM command issuer derived from wbz451_issue_nvmcmd */
// static int pic32cx_issue_nvmcmd(const struct pic32cx_dev *dev, struct target *t,
//                                 uint32_t flash_addr, uint16_t cmd, uint32_t src_addr)
// {
//     int res;

//     /* For boot/alias region operations we must write NVMLBWP */
//     if (flash_addr < dev->flash_base) {
//         if (dev->nvmlbwp) {
//             res = target_write_u32(t, dev->nvmlbwp, NVMLBWP_UNLOCK_KEY);
//             if (res != ERROR_OK) return res;
//         }
//     } else {
//         res = pic32cx_unlock_flash(dev, t);
//         if (res != ERROR_OK) return res;
//     }

//     /* Clear previous error flags */
//     if (dev->nvmconclr) {
//         res = target_write_u32(t, dev->nvmconclr, dev->nvm_err_mask);
//         if (res != ERROR_OK) return res;
//     }

//     /* Align dest to row */
//     flash_addr &= ~(dev->row_size - 1U);

//     /* Set NVMSRCADDR if ROW_PROGRAM */
//     if (cmd == NVMCON_OP_ROW_PROG && dev->nvmsrcaddr) {
//         res = target_write_u32(t, dev->nvmsrcaddr, src_addr);
//         if (res != ERROR_OK) return res;
//     }

//     /* Set NVMADDR */
//     res = target_write_u32(t, dev->nvmaddr, flash_addr);
//     if (res != ERROR_OK) return res;

//     /* Set WREN and operation */
//     res = target_write_u32(t, dev->nvmcon, NVMCON_NVMWREN | (cmd & NVMCON_OP_MASK));
//     if (res != ERROR_OK) return res;

//     /* NVMKEY sequence */
//     res = target_write_u32(t, dev->nvmkey, 0x00000000U); if (res != ERROR_OK) return res;
//     res = target_write_u32(t, dev->nvmkey, 0xAA996655U); if (res != ERROR_OK) return res;
//     res = target_write_u32(t, dev->nvmkey, 0x556699AAU); if (res != ERROR_OK) return res;

//     /* Start operation */
//     res = target_write_u32(t, dev->nvmconset, NVMCON_NVMWR);
//     if (res != ERROR_OK) return res;

//     /* Wait for NVMWR to clear */
//     uint32_t val;
//     int timeout = 10000;
//     do {
//         res = target_read_u32(t, dev->nvmcon, &val);
//         if (res != ERROR_OK) return res;
//         if (!--timeout) {
//             LOG_ERROR("Timeout waiting for NVMWR clear (addr: 0x%08" PRIx32 ", cmd: 0x%X)", flash_addr, cmd);
//             return ERROR_FAIL;
//         }
//         alive_sleep(1);
//     } while (val & NVMCON_NVMWR);

//     if (val & dev->nvm_err_mask) {
//         LOG_ERROR("NVM error detected (NVMCON=0x%08" PRIx32 ")", val);
//         return ERROR_FAIL;
//     }

//     /* Clear NVMWREN */
//     return target_write_u32(t, dev->nvmconclr, NVMCON_NVMWREN);
// }

// /* Probe: uses per-device probe logic (keeps original region checks) */
// static int pic32cx_probe(struct flash_bank *bank)
// {
//     struct pic32cx_priv *priv = bank->driver_priv;
//     if (!priv) return ERROR_FAIL;
//     if (priv->probed) return ERROR_OK;

//     struct target *t = bank->target;
//     uint32_t did = 0;
//     int res = target_read_u32(t, DSU_DID_REG, &did);

//     const struct pic32cx_dev *dev = NULL;
//     if (res == ERROR_OK) {
//         dev = find_device_by_did(did);
//         if (dev)
//             LOG_INFO("Detected device by DSU DID: %s (DID 0x%08" PRIx32 ")", dev->name, did);
//     } else {
//         LOG_WARNING("Failed to read DSU DID at 0x%08" PRIx32 ". Will try name-based detection.", DSU_DID_REG);
//     }

//     /* name fallback */
//     if (!dev)
//         dev = find_device_by_name(bank->name);

//     if (!dev) {
//         LOG_WARNING("No device matched by DID or name. Falling back to WBZ451 defaults.");
//         dev = &device_table[0];
//     }

//     priv->dev = dev;
//     priv->target = t;

//     /* Use device-specific probe ranges — reuse logic from original files */
//     uint32_t base = bank->base;
//     uint32_t param = 0;
//     if (dev->nvm_param)
//         target_read_u32(t, dev->nvm_param, &param);

//     /* Apply device-specific mapping (copied/adapted from originals) */
//     if ((strcmp(dev->name, "WBZ451") == 0) || (strcmp(dev->name, "WBZ450") == 0) || (strcmp(dev->name, "WBZ451H") == 0)) {
//         if (base < 0x00005000U) {
//             priv->page_size = dev->row_size;
//             priv->num_pages = 20; /* 20 kB BootFlash */
//         } else if (base >= 0x00005000U && base < 0x00006000U) {
//             priv->page_size = dev->row_size; priv->num_pages = 4;
//         } else if (base >= 0x00006000U && base < 0x00007000U) {
//             priv->page_size = dev->row_size; priv->num_pages = 4;
//         } else if (base >= 0x00045000U && base < 0x00047000U) {
//             priv->page_size = dev->row_size; priv->num_pages = 8;
//         } else if (base >= dev->flash_base && base < (dev->flash_base + dev->flash_size)) {
//             priv->page_size = dev->row_size; priv->num_pages = dev->flash_size / dev->row_size;
//         } else if (bank->base == 0xE000ED10U) {
//             priv->page_size = dev->row_size; priv->num_pages = 1;
//         }
//     } else if ((strcmp(dev->name, "WBZ351") == 0) || (strcmp(dev->name, "WBZ350") == 0)) {
//         /* mapping from wbz351.c */
//         if (base < 0x00010000U) {
//             priv->page_size = dev->row_size; priv->num_pages = 64;
//         } else if (bank->base >= 0x00800000U && base < 0x00805000U) {
//             priv->page_size = dev->row_size; priv->num_pages = 20;
//         } else if (bank->base >= 0x00805000U && bank->base < 0x00806000U) {
//             priv->page_size = dev->row_size; priv->num_pages = 4;
//         } else if (bank->base >= 0x00806000U && bank->base < 0x00827000U) {
//             priv->page_size = dev->row_size; priv->num_pages = 132;
//         } else if (bank->base >= dev->flash_base && bank->base < (dev->flash_base + dev->flash_size)) {
//             priv->page_size = dev->row_size; priv->num_pages = dev->flash_size / dev->row_size;
//         } else if (bank->base == 0xE000ED10U) {
//             priv->page_size = dev->row_size; priv->num_pages = 1;
//         }
//     } else if (strcmp(dev->name, "PIC32WM") == 0) { /* PIC32WM_BZ6204 mapping similar to pic32wm.c */
//         if (base < 0x00010000U) {
//             priv->page_size = dev->row_size; priv->num_pages = 64;
//         } else if (bank->base >= 0x00800000U && bank->base < 0x00810000U) {
//             priv->page_size = dev->row_size; priv->num_pages = 64;
//         } else if (bank->base >= 0x00810000U && bank->base < 0x00811000U) {
//             priv->page_size = dev->row_size; priv->num_pages = 4;
//         } else if (bank->base >= 0x00811000U && bank->base < 0x00812000U) {
//             priv->page_size = dev->row_size; priv->num_pages = 4;
//         } else if (bank->base >= dev->flash_base && bank->base < (dev->flash_base + dev->flash_size)) {
//             priv->page_size = dev->row_size; priv->num_pages = dev->flash_size / dev->row_size;
//         } else if (bank->base == 0xE000ED10U) {
//             priv->page_size = dev->row_size; priv->num_pages = 1;
//         }
//     }

//     /* if not set above, fall back to a single page of row_size */
//     if (priv->page_size == 0) {
//         priv->page_size = dev->row_size;
//         priv->num_pages = (dev->flash_size / dev->row_size);
//     }

//     bank->size = priv->page_size * priv->num_pages;
//     bank->num_sectors = priv->num_pages;

//     if (bank->sectors) {
//         free(bank->sectors);
//         bank->sectors = NULL;
//     }

//     bank->sectors = calloc(bank->num_sectors, sizeof(struct flash_sector));
//     if (!bank->sectors) return ERROR_FAIL;

//     for (unsigned int i = 0; i < bank->num_sectors; ++i) {
//         bank->sectors[i].offset = i * priv->page_size;
//         bank->sectors[i].size = priv->page_size;
//         bank->sectors[i].is_protected = 0;
//     }

//     priv->probed = true;
//     LOG_INFO("%s: probe complete. base=0x%08" PRIx64 " size=0x%08" PRIx32 " pages=%" PRIu32,
//          dev->name, (uint64_t)bank->base, bank->size, priv->num_pages);

//     return ERROR_OK;
// }

// /* Erase: page erase (acts on sectors) */
// static int pic32cx_erase(struct flash_bank *bank, unsigned int first, unsigned int last)
// {
//     struct pic32cx_priv *p = bank->driver_priv;
//     if (!p) return ERROR_FAIL;

//     struct target *t = bank->target;
//     if (t->state != TARGET_HALTED) return ERROR_TARGET_NOT_HALTED;

//     const struct pic32cx_dev *dev = p->dev;

//     for (unsigned int i = first; i <= last; ++i) {
//         uint32_t addr = bank->base + i * p->page_size;
//         int res = pic32cx_issue_nvmcmd(dev, t, addr, NVMCON_OP_PAGE_ERASE, 0);
//         if (res != ERROR_OK) return res;
//     }
//     return ERROR_OK;
// }

// /* Write: row-by-row write (same pattern as originals) */
// static int pic32cx_write(struct flash_bank *bank, const uint8_t *buf, uint32_t offset, uint32_t count)
// {
//     struct pic32cx_priv *p = bank->driver_priv;
//     if (!p) return ERROR_FAIL;

//     struct target *target = bank->target;
//     if (target->state != TARGET_HALTED) {
//         LOG_ERROR("Target not halted");
//         return ERROR_TARGET_NOT_HALTED;
//     }

//     const struct pic32cx_dev *dev = p->dev;
//     uint32_t row_size = dev->row_size;
//     uint32_t addr = bank->base + offset;
//     const uint32_t end = addr + count;

//     for (uint32_t row_addr = addr & ~(row_size - 1U); row_addr < end; row_addr += row_size) {
//         uint8_t *row_buf = malloc(row_size);
//         if (!row_buf) {
//             LOG_ERROR("Out of memory allocating row buffer");
//             return ERROR_FAIL;
//         }

//         int res = target_read_memory(target, row_addr, 4, row_size / 4, row_buf);
//         if (res != ERROR_OK) {
//             LOG_ERROR("Failed to read flash row at 0x%08" PRIx32, row_addr);
//             free(row_buf);
//             return res;
//         }

//         for (uint32_t i = 0; i < row_size; ++i) {
//             uint32_t abs_addr = row_addr + i;
//             if (abs_addr >= addr && abs_addr < end) row_buf[i] = buf[abs_addr - addr];
//         }

//         res = target_write_buffer(target, RAM_BUF_ADDR, row_size, row_buf);
//         if (res != ERROR_OK) {
//             LOG_ERROR("Failed to write row buffer to RAM at 0x%08" PRIx32, (uint32_t)RAM_BUF_ADDR);
//             free(row_buf);
//             return res;
//         }

//         res = pic32cx_issue_nvmcmd(dev, target, row_addr, NVMCON_OP_ROW_PROG, RAM_BUF_ADDR);
//         if (res != ERROR_OK) {
//             LOG_ERROR("Failed to program row at 0x%08" PRIx32, row_addr);
//             free(row_buf);
//             return res;
//         }

//         alive_sleep(2);
//         free(row_buf);
//     }

//     LOG_INFO("%s: Write completed", p->dev->name);
//     return ERROR_OK;
// }

// /* dsu_reset_deassert - tries to find a bank name (keeps compatibility) */
// COMMAND_HANDLER(pic32cx_handle_dsu_reset_deassert)
// {
//     struct target *target = get_current_target(CMD_CTX);
//     if (!target)
//         return ERROR_FAIL;

//     struct flash_bank *bank = get_flash_bank_by_num_noprobe(0);
//     if (!bank)
//         return ERROR_FAIL;

//     target_write_u8(target, 0x44000001U, 0x02U);
//     target_write_u32(target, 0x44000100U, 0x8000U);
//     target_write_u32(target, 0xE000ED0CU, 0x05FA0004U);
//     target_write_u32(target, 0xE000ED0CU, 0x05FA0000U);
//     alive_sleep(100);
//     return ERROR_OK;
// }

// /* flash bank attach handler */
// FLASH_BANK_COMMAND_HANDLER(pic32cx_flash_bank_command)
// {
//     struct pic32cx_priv *chip = calloc(1, sizeof(*chip));
//     if (!chip) return ERROR_FAIL;
//     chip->target = bank->target;
//     chip->probed = false;
//     chip->dev = &device_table[0]; /* default */
//     chip->page_size = device_table[0].page_size;
//     bank->driver_priv = chip;
//     return ERROR_OK;
// }

// /* erase_page and write_word wrappers that operate using first table device (manual exec) */
// COMMAND_HANDLER(pic32cx_handle_erase_page_command)
// {
//     struct target *target = get_current_target(CMD_CTX);
//     if (!target || CMD_ARGC != 1)
//         return ERROR_COMMAND_SYNTAX_ERROR;

//     uint32_t addr;
//     COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], addr);

//     struct flash_bank *bank = get_flash_bank_by_num_noprobe(0);
//     if (!bank)
//         return ERROR_FAIL;

//     struct pic32cx_priv *p = bank->driver_priv;
//     const struct pic32cx_dev *dev = (p && p->dev) ? p->dev : &device_table[0];

//     return pic32cx_issue_nvmcmd(dev, target, addr, NVMCON_OP_PAGE_ERASE, 0);
// }

// COMMAND_HANDLER(pic32cx_handle_write_word_command)
// {
//     struct target *target = get_current_target(CMD_CTX);
//     if (!target || CMD_ARGC != 2)
//         return ERROR_COMMAND_SYNTAX_ERROR;

//     uint32_t addr, value;
//     COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], addr);
//     COMMAND_PARSE_NUMBER(u32, CMD_ARGV[1], value);

//     struct flash_bank *bank = get_flash_bank_by_num_noprobe(0);
//     if (!bank)
//         return ERROR_FAIL;

//     struct pic32cx_priv *p = bank->driver_priv;
//     const struct pic32cx_dev *dev = (p && p->dev) ? p->dev : &device_table[0];

//     target_write_u32(target, dev->nvmdata, value);
//     return pic32cx_issue_nvmcmd(dev, target, addr, NVMCON_OP_WORD_PROG, 0);
// }

// /* command array registration */
// static const struct command_registration pic32cx_exec_command_handlers[] = {
//     {
//         .name = "erase_page",
//         .handler = pic32cx_handle_erase_page_command,
//         .mode = COMMAND_EXEC,
//         .usage = "<address>",
//         .help = "Erase a flash page at the given address",
//     },
//     {
//         .name = "write_word",
//         .handler = pic32cx_handle_write_word_command,
//         .mode = COMMAND_EXEC,
//         .usage = "<address> <32bit_hex_value>",
//         .help = "Write a 32-bit word to flash at the given address",
//     },
//     {
//         .name = "dsu_reset_deassert",
//         .handler = pic32cx_handle_dsu_reset_deassert,
//         .mode = COMMAND_EXEC,
//         .usage = "",
//         .help = "Device-specific DSU reset deassert sequence",
//     },
//     COMMAND_REGISTRATION_DONE
// };

// static const struct command_registration pic32cx_command_handlers[] = {
//     {
//         .name = "pic32cx_bz",
//         .mode = COMMAND_ANY,
//         .help = "pic32cx_bz flash command group",
//         .usage = "",
//         .chain = pic32cx_exec_command_handlers,
//     },
//     COMMAND_REGISTRATION_DONE
// };

// const struct flash_driver pic32cx_bz_flash = {
//     .name                = "pic32cx_bz",
//     .commands            = pic32cx_command_handlers,
//     .flash_bank_command  = pic32cx_flash_bank_command,
//     .erase               = pic32cx_erase,
//     .protect             = NULL,
//     .write               = pic32cx_write,
//     .read                = default_flash_read,
//     .probe               = pic32cx_probe,
//     .auto_probe          = pic32cx_probe,
//     .erase_check         = default_flash_blank_check,
//     .protect_check       = NULL,
//     .free_driver_priv    = default_flash_free_driver_priv,
// };









// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Microchip Technology Inc.
 * Author: Dinesh Arasu <dinesh.arasu@microchip.com>
 *
 * Description: Flash driver for WBZ and PIC32CX_BZx Microchip Curiosity Board.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "imp.h"
#include "helper/binarybuffer.h"
#include "helper/time_support.h"
#include <jtag/jtag.h>
#include <target/cortex_m.h>
#include <target/target_type.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#define DSU_DID_REG 0x41000018U

/* Generic NVM operation bits */
#define NVMCON_NVMWREN          BIT(14)
#define NVMCON_NVMWR            BIT(15)
#define NVMCON_OP_WORD_PROG     0x4001
#define NVMCON_OP_ROW_PROG      0x4003
#define NVMCON_OP_PAGE_ERASE    0x4004
#define NVMCON_OP_PBC           0x4007
#define NVMCON_OP_MASK          GENMASK(14, 0)

/* RAM staging buffer address used across drivers */
#define RAM_BUF_ADDR 0x20000000U

/* default row size used by these parts */
#define DEFAULT_ROW_SIZE 1024U

/* NVMLBWP unlock key used in originals */
#define NVMLBWP_UNLOCK_KEY 0x80000000U

#define NVM_CMD_TIMEOUT_MS 10000
#define NVM_POLL_DELAY_MS 1
#define NVM_ROW_DELAY_MS 5

/* PAC register offsets and bits */
#define PAC_STATUS_OFFSET       0x18U
#define PAC_UNLOCK_OFFSET       0x20U
#define PAC_NVMCTRL_BIT         BIT(1)

/* DSU and reset registers */
#define DSU_CTRL_REG            0x44000001U
#define DSU_CTRL_CPU_RESET_BIT  BIT(1)
#define DSU_RESET_REG           0x44000100U
#define DSU_RESET_KEY           0x8000U
#define AIRCR_REG               0xE000ED0CU
#define AIRCR_SYS_RESET_REQ     0x05FA0004U
#define AIRCR_VECT_KEY          0x05FA0000U
#define DSU_RESET_DELAY_MS      100

static const uint32_t pic32cx_nvmkey[] = {
	0x00000000U,
	0xAA996655U,
	0x556699AAU,
};

struct pic32cx_map {
	uint32_t start;
	uint32_t end;
	uint32_t page_size;
	uint32_t num_pages;
};

/* -------------------------------------------------------------------------- */
/* WBZ451 / WBZ450 mapping */
static const struct pic32cx_map map_wbz45x[] = {
	{ .start = 0x00000000U, .end = 0x00005000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 20 },
	{ .start = 0x00005000U, .end = 0x00006000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 4 },
	{ .start = 0x00006000U, .end = 0x00007000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 4 },
	{ .start = 0x00045000U, .end = 0x00047000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 8 },
	{ .start = 0x01000000U, .end = 0x01000000U + 0x00100000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 0x00100000U / DEFAULT_ROW_SIZE },
	{ .start = 0xE000ED10U, .end = 0xE000ED10U + 4U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 1 },
};

/* WBZ351 / WBZ350 mapping */
static const struct pic32cx_map map_wbz35x[] = {
	{ .start = 0x00000000U, .end = 0x00010000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 64 },
	{ .start = 0x00800000U, .end = 0x00805000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 20 },
	{ .start = 0x00805000U, .end = 0x00806000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 4 },
	{ .start = 0x00806000U, .end = 0x00827000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 132 },
	{ .start = 0x01000000U, .end = 0x01000000U + 0x00080000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 0x00080000U / DEFAULT_ROW_SIZE },
	{ .start = 0xE000ED10U, .end = 0xE000ED10U + 4U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 1 },
};

/* PIC32WM_BZ6204 mapping */
static const struct pic32cx_map map_pic32wm[] = {
	{ .start = 0x00000000U, .end = 0x00010000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 64 },
	{ .start = 0x00800000U, .end = 0x00810000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 64 },
	{ .start = 0x00810000U, .end = 0x00811000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 4 },
	{ .start = 0x00811000U, .end = 0x00812000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 4 },
	{ .start = 0x01000000U, .end = 0x01000000U + 0x00200000U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 0x00200000U / DEFAULT_ROW_SIZE },
	{ .start = 0xE000ED10U, .end = 0xE000ED10U + 4U, .page_size = DEFAULT_ROW_SIZE, .num_pages = 1 },
};

struct pic32cx_dev
{
	uint32_t device_id;
	const char *name;
	uint32_t flash_base;
	uint32_t flash_size;
	uint32_t row_size;
	uint32_t page_size;
	uint32_t pac_base;
	uint32_t nvmcon;
	uint32_t nvmkey;
	uint32_t nvmaddr;
	uint32_t nvmsrcaddr;
	uint32_t nvmdata;
	uint32_t nvmconset;
	uint32_t nvmconclr;
	uint32_t nvmlbwp;
	uint32_t nvm_param;
	uint32_t nvm_err_mask;
	const struct pic32cx_map *map;
	size_t map_length;
};

static const struct pic32cx_dev device_table[] = {
	{
		.device_id    = 0x00009B8F, /* WBZ451 */
		.name         = "wbz451",
		.flash_base   = 0x01000000U,
		.flash_size   = 0x00100000U, /* 1 MB */
		.row_size     = DEFAULT_ROW_SIZE,
		.page_size    = DEFAULT_ROW_SIZE,
		.pac_base     = 0x40000000U,
		.nvmcon       = 0x44000600U,
		.nvmkey       = 0x44000620U,
		.nvmaddr      = 0x44000630U,
		.nvmsrcaddr   = 0x440006C0U,
		.nvmdata      = 0x44000640U,
		.nvmconset    = 0x44000608U,
		.nvmconclr    = 0x44000604U,
		.nvmlbwp      = 0x440006F0U,
		.nvm_param    = 0x44000610U,
		.nvm_err_mask = GENMASK(13, 8),
		.map          = map_wbz45x,
		.map_length   = ARRAY_SIZE(map_wbz45x),
	},
	{
		.device_id    = 0x00009B0B, /* WBZ450 */
		.name         = "wbz450",
		.flash_base   = 0x01000000U,
		.flash_size   = 0x00100000U, /* 1 MB */
		.row_size     = DEFAULT_ROW_SIZE,
		.page_size    = DEFAULT_ROW_SIZE,
		.pac_base     = 0x40000000U,
		.nvmcon       = 0x44000600U,
		.nvmkey       = 0x44000620U,
		.nvmaddr      = 0x44000630U,
		.nvmsrcaddr   = 0x440006C0U,
		.nvmdata      = 0x44000640U,
		.nvmconset    = 0x44000608U,
		.nvmconclr    = 0x44000604U,
		.nvmlbwp      = 0x440006F0U,
		.nvm_param    = 0x44000610U,
		.nvm_err_mask = GENMASK(13, 8),
		.map          = map_wbz45x,
		.map_length   = ARRAY_SIZE(map_wbz45x),
	},
	{
	.device_id    = 0, /* WBZ451H */
	.name         = "wbz451hpe",
	.flash_base   = 0x01000000U,
	.flash_size   = 0x00100000U, /* 1 MB */
	.row_size     = DEFAULT_ROW_SIZE,
	.page_size    = DEFAULT_ROW_SIZE,
	.pac_base     = 0x40000000U,
	.nvmcon       = 0x44000600U,
	.nvmkey       = 0x44000620U,
	.nvmaddr      = 0x44000630U,
	.nvmsrcaddr   = 0x440006C0U,
	.nvmdata      = 0x44000640U,
	.nvmconset    = 0x44000608U,
	.nvmconclr    = 0x44000604U,
	.nvmlbwp      = 0x440006F0U,
	.nvm_param    = 0x44000610U,
	.nvm_err_mask = GENMASK(13, 8),
	.map          = map_wbz45x,
	.map_length   = ARRAY_SIZE(map_wbz45x),
},
	{
		.device_id    = 0x00009E03, /* WBZ351 */
		.name         = "wbz351",
		.flash_base   = 0x01000000U,
		.flash_size   = 0x00080000U, /* 512 KB */
		.row_size     = DEFAULT_ROW_SIZE,
		.page_size    = DEFAULT_ROW_SIZE,
		.pac_base     = 0x40000000U,
		.nvmcon       = 0x44000600U,
		.nvmkey       = 0x44000620U,
		.nvmaddr      = 0x44000630U,
		.nvmsrcaddr   = 0x440006C0U,
		.nvmdata      = 0x44000640U,
		.nvmconset    = 0x44000608U,
		.nvmconclr    = 0x44000604U,
		.nvmlbwp      = 0x440006F0U,
		.nvm_param    = 0x44000610U,
		.nvm_err_mask = GENMASK(13, 8),
		.map          = map_wbz35x,
		.map_length   = ARRAY_SIZE(map_wbz35x),
	},
	{
		.device_id    = 0x0001A800, /* PIC32WM_BZ6204 */
		.name         = "pic32wm",
		.flash_base   = 0x01000000U,
		.flash_size   = 0x00200000U, /* 2 MB */
		.row_size     = DEFAULT_ROW_SIZE,
		.page_size    = DEFAULT_ROW_SIZE,
		.pac_base     = 0x40000000U,
		.nvmcon       = 0x44000600U,
		.nvmkey       = 0x44000620U,
		.nvmaddr      = 0x44000630U,
		.nvmsrcaddr   = 0x440006C0U,
		.nvmdata      = 0x44000640U,
		.nvmconset    = 0x44000608U,
		.nvmconclr    = 0x44000604U,
		.nvmlbwp      = 0x440006F0U,
		.nvm_param    = 0x44000610U,
		.nvm_err_mask = GENMASK(13, 8),
		.map          = map_pic32wm,
		.map_length   = ARRAY_SIZE(map_pic32wm),
	}
};

struct pic32cx_priv
{
	struct target *target;
	bool probed;
	const struct pic32cx_dev *dev;
	uint32_t page_size;
	uint32_t num_pages;
};

static const struct pic32cx_dev *find_device_by_did(uint32_t did)
{
	for (size_t i = 0; i < ARRAY_SIZE(device_table); ++i)
		if (device_table[i].device_id != 0 && device_table[i].device_id == did)
			return &device_table[i];
	return NULL;
}

static const struct pic32cx_dev *find_device_by_name(const char *name)
{
	if (!name)
		return NULL;
	for (size_t i = 0; i < ARRAY_SIZE(device_table); ++i)
		if (strstr(name, device_table[i].name))
			return &device_table[i];
	return NULL;
}

/* unlock via PAC if locked (same as originals) */
static int pic32cx_unlock_flash(const struct pic32cx_dev *dev, struct target *t)
{
	uint32_t status;
	int res = target_read_u32(t, dev->pac_base + PAC_STATUS_OFFSET, &status);
	if (res != ERROR_OK)
		return res;

	if (status & PAC_NVMCTRL_BIT) {
		LOG_INFO("PAC indicates NVMCTRL is locked. Attempting to unlock...");
		res = target_write_u32(t, dev->pac_base + PAC_UNLOCK_OFFSET, PAC_NVMCTRL_BIT);
		if (res != ERROR_OK) return res;
		alive_sleep(5); /* small settle */
	}
	return target_read_u32(t, dev->pac_base + PAC_STATUS_OFFSET, &status);
}

/* Generic NVM command issuer derived from wbz451_issue_nvmcmd */
static int pic32cx_issue_nvmcmd(const struct pic32cx_dev *dev, struct target *t,
								uint32_t flash_addr, uint16_t cmd, uint32_t src_addr)
{
	int res;

	/* For boot/alias region operations we must write NVMLBWP */
	if (flash_addr < dev->flash_base) {
		if (dev->nvmlbwp) {
			res = target_write_u32(t, dev->nvmlbwp, NVMLBWP_UNLOCK_KEY);
			if (res != ERROR_OK)
				return res;
		}
	} else {
	res = pic32cx_unlock_flash(dev, t);
	if (res != ERROR_OK)
		return res;
	}

	/* Clear previous error flags */
	if (dev->nvmconclr) {
		// res = target_write_u32(t, dev->nvmconclr, dev->nvm_err_mask);
		uint32_t clear_mask = (dev->nvm_err_mask & 0x3F00U);
		res = target_write_u32(t, dev->nvmconclr, clear_mask);
		if (res != ERROR_OK) return res;
	}

	/* Align dest to row */
	flash_addr &= ~(dev->row_size - 1U);

	/* Set NVMSRCADDR if ROW_PROGRAM */
	if (cmd == NVMCON_OP_ROW_PROG && dev->nvmsrcaddr)
	{
		res = target_write_u32(t, dev->nvmsrcaddr, src_addr);
		if (res != ERROR_OK)
			return res;
	}

	/* Set NVMADDR */
	res = target_write_u32(t, dev->nvmaddr, flash_addr);
	if (res != ERROR_OK)
		return res;

	/* Set WREN and operation */
	res = target_write_u32(t, dev->nvmcon, NVMCON_NVMWREN | (cmd & NVMCON_OP_MASK));
	if (res != ERROR_OK)
		return res;

	/* NVMKEY sequence */
	for (size_t i = 0; i < ARRAY_SIZE(pic32cx_nvmkey); i++) {
	res = target_write_u32(t, dev->nvmkey, pic32cx_nvmkey[i]);
	if (res != ERROR_OK)
		return res;
}

	/* Start operation */
	res = target_write_u32(t, dev->nvmconset, NVMCON_NVMWR);
	if (res != ERROR_OK)
		return res;

	/* Wait for NVMWR to clear */
	uint32_t val;
	long long start = timeval_ms();
	do {
		res = target_read_u32(t, dev->nvmcon, &val);

		if (res != ERROR_OK)
			return res;

		if ((timeval_ms() - start) > NVM_CMD_TIMEOUT_MS) {
		LOG_ERROR("Timeout waiting for NVMWR clear (addr: 0x%08" PRIx32 ", cmd: 0x%X)", flash_addr, cmd);
		/* Ensure WREN cleared before exit */
			(void)target_write_u32(t, dev->nvmconclr, NVMCON_NVMWREN);
		return ERROR_FAIL;
	}

	alive_sleep(NVM_POLL_DELAY_MS);
	} while (val & NVMCON_NVMWR);

	/* Final explicit read to capture any late-set error bits */
	res = target_read_u32(t, dev->nvmcon, &val);
	if (res != ERROR_OK) {
		(void)target_write_u32(t, dev->nvmconclr, NVMCON_NVMWREN);
		return res;
	}

	if (val & dev->nvm_err_mask) {
		LOG_ERROR("NVM error detected (NVMCON=0x%08" PRIx32 ")", val);
		/* Clear WREN before returning error */
		(void)target_write_u32(t, dev->nvmconclr, NVMCON_NVMWREN);
		return ERROR_FAIL;
	}

	/* Clear NVMWREN */
	return target_write_u32(t, dev->nvmconclr, NVMCON_NVMWREN);
}

/* Probe: uses per-device probe logic (keeps original region checks) */
static int pic32cx_probe(struct flash_bank *bank)
{
	struct pic32cx_priv *priv = bank->driver_priv;
	if (!priv)
		return ERROR_FAIL;
	if (priv->probed)
		return ERROR_OK;

	struct target *t = bank->target;
	uint32_t did = 0;
	int res = target_read_u32(t, DSU_DID_REG, &did);

	const struct pic32cx_dev *dev = NULL;
	if (res == ERROR_OK) {
		dev = find_device_by_did(did);
		if (dev)
			LOG_INFO("Detected device by DSU DID: %s (DID 0x%08" PRIx32 ")", dev->name, did);
	} else {
		LOG_WARNING("Failed to read DSU DID at 0x%08" PRIx32 ". Will try name-based detection.", DSU_DID_REG);
	}

	/* name fallback */
	if (!dev)
		dev = find_device_by_name(bank->name);

	if (!dev) {
		LOG_WARNING("No device matched by DID or name. Falling back to WBZ451 defaults.");
		dev = &device_table[0];
	}

	priv->dev = dev;
	priv->target = t;

	/* Use device-specific probe ranges — reuse logic from original files */
	uint32_t base = bank->base;
	uint32_t param = 0;
	if (dev->nvm_param)
		target_read_u32(t, dev->nvm_param, &param);

	/* Find matching map entry */
	if (dev->map && dev->map_length) {
		for (size_t i = 0; i < dev->map_length; ++i) {
			const struct pic32cx_map *m = &dev->map[i];
			if (base >= m->start && base < m->end) {
				priv->page_size = m->page_size;
				priv->num_pages = m->num_pages;
				break;
			}
		}
	}

	/* if not set above, fall back to a single page of row_size */
	if (priv->page_size == 0) {
		priv->page_size = dev->row_size;
		priv->num_pages = (dev->flash_size / dev->row_size);
	}

	bank->size = priv->page_size * priv->num_pages;
	bank->num_sectors = priv->num_pages;

	free(bank->sectors);
	bank->sectors = NULL;

	bank->sectors = calloc(bank->num_sectors, sizeof(struct flash_sector));
	if (!bank->sectors)
		return ERROR_FAIL;

	for (unsigned int i = 0; i < bank->num_sectors; ++i) {
		bank->sectors[i].offset = i * priv->page_size;
		bank->sectors[i].size = priv->page_size;
		bank->sectors[i].is_protected = 0;
	}

	priv->probed = true;
	LOG_INFO("%s: probe complete. base=0x%08" PRIx64 " size=0x%08" PRIx32 " pages=%" PRIu32,
		dev->name, (uint64_t)bank->base, bank->size, priv->num_pages);

	return ERROR_OK;
}

/* Erase: page erase (acts on sectors) */
static int pic32cx_erase(struct flash_bank *bank, unsigned int first, unsigned int last)
{
	struct pic32cx_priv *p = bank->driver_priv;
	if (!p)
		return ERROR_FAIL;

	struct target *t = bank->target;
	if (t->state != TARGET_HALTED) return ERROR_TARGET_NOT_HALTED;

	const struct pic32cx_dev *dev = p->dev;

	for (unsigned int i = first; i <= last; ++i) {
		uint32_t addr = bank->base + i * p->page_size;
		int res = pic32cx_issue_nvmcmd(dev, t, addr, NVMCON_OP_PAGE_ERASE, 0);
		if (res != ERROR_OK)
			return res;
		alive_sleep(NVM_ROW_DELAY_MS); /* small pause between erase ops */
	}
	return ERROR_OK;
}

/* Full-chip erase using PBC command (manual command) */
static int pic32cx_erase_chip(struct flash_bank *bank)
{
	struct pic32cx_priv *p = bank->driver_priv;
	if (!p)
		return ERROR_FAIL;

	struct target *t = bank->target;
	if (t->state != TARGET_HALTED)
		return ERROR_TARGET_NOT_HALTED;

	const struct pic32cx_dev *dev = p->dev;
	LOG_INFO("%s: Performing full chip erase (PBC)...", dev->name);

	int res = pic32cx_issue_nvmcmd(dev, t, dev->flash_base, NVMCON_OP_PBC, 0);
	alive_sleep(10); /* allow extra settle after PBC */
	return res;
}

/* Write: row-by-row write (same pattern as originals) */
static int pic32cx_write(struct flash_bank *bank, const uint8_t *buf, uint32_t offset, uint32_t count)
{
	struct pic32cx_priv *p = bank->driver_priv;
	if (!p)
		return ERROR_FAIL;

	struct target *target = bank->target;
	if (target->state != TARGET_HALTED) {
		LOG_ERROR("Target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	const struct pic32cx_dev *dev = p->dev;
	uint32_t row_size = dev->row_size;
	uint32_t addr = bank->base + offset;
	const uint32_t end = addr + count;

	for (uint32_t row_addr = addr & ~(row_size - 1U); row_addr < end; row_addr += row_size) {
		uint8_t *row_buf = calloc(1, row_size);
		if (!row_buf) {
			LOG_ERROR("Out of memory allocating row buffer");
			return ERROR_FAIL;
		}

		int res = target_read_memory(target, row_addr, 4, row_size / 4, row_buf);
		if (res != ERROR_OK) {
			LOG_ERROR("Failed to read flash row at 0x%08" PRIx32, row_addr);
			free(row_buf);
			return res;
		}

		for (uint32_t i = 0; i < row_size; ++i) {
			uint32_t abs_addr = row_addr + i;
			if (abs_addr >= addr && abs_addr < end) row_buf[i] = buf[abs_addr - addr];
		}

		res = target_write_buffer(target, RAM_BUF_ADDR, row_size, row_buf);
		if (res != ERROR_OK) {
			LOG_ERROR("Failed to write row buffer to RAM at 0x%08" PRIx32, (uint32_t)RAM_BUF_ADDR);
			free(row_buf);
			return res;
		}

		res = pic32cx_issue_nvmcmd(dev, target, row_addr, NVMCON_OP_ROW_PROG, RAM_BUF_ADDR);
		if (res != ERROR_OK) {
			LOG_ERROR("Failed to program row at 0x%08" PRIx32, row_addr);
			free(row_buf);
			return res;
		}

		alive_sleep(NVM_ROW_DELAY_MS);
		free(row_buf);
	}

	LOG_INFO("%s: Write completed", p->dev->name);
	return ERROR_OK;
}

/* dsu_reset_deassert - tries to find a bank name (keeps compatibility) */
COMMAND_HANDLER(pic32cx_handle_dsu_reset_deassert)
{
	struct target *target = get_current_target(CMD_CTX);
	if (!target)
		return ERROR_FAIL;

	struct flash_bank *bank = get_flash_bank_by_num_noprobe(0);
	if (!bank)
		return ERROR_FAIL;

	target_write_u8(target, DSU_CTRL_REG, DSU_CTRL_CPU_RESET_BIT);
	target_write_u32(target, DSU_RESET_REG, DSU_RESET_KEY);
	target_write_u32(target, AIRCR_REG, AIRCR_SYS_RESET_REQ);
	target_write_u32(target, AIRCR_REG, AIRCR_VECT_KEY);
	alive_sleep(DSU_RESET_DELAY_MS);
	return ERROR_OK;
}

/* flash bank attach handler */
FLASH_BANK_COMMAND_HANDLER(pic32cx_flash_bank_command)
{
	struct pic32cx_priv *chip = calloc(1, sizeof(*chip));
	if (!chip) return ERROR_FAIL;
	chip->target = bank->target;
	chip->probed = false;
	chip->dev = &device_table[0]; /* default */
	chip->page_size = device_table[0].page_size;
	bank->driver_priv = chip;
	return ERROR_OK;
}

/* erase_page and write_word wrappers that operate using first table device (manual exec) */
COMMAND_HANDLER(pic32cx_handle_erase_page_command)
{
	struct target *target = get_current_target(CMD_CTX);
	if (!target || CMD_ARGC != 1)
		return ERROR_COMMAND_SYNTAX_ERROR;

	uint32_t addr;
	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], addr);

	struct flash_bank *bank = get_flash_bank_by_num_noprobe(0);
	if (!bank)
		return ERROR_FAIL;

	struct pic32cx_priv *p = bank->driver_priv;
	const struct pic32cx_dev *dev = (p && p->dev) ? p->dev : &device_table[0];

	return pic32cx_issue_nvmcmd(dev, target, addr, NVMCON_OP_PAGE_ERASE, 0);
}

COMMAND_HANDLER(pic32cx_handle_write_word_command)
{
	struct target *target = get_current_target(CMD_CTX);
	if (!target || CMD_ARGC != 2)
		return ERROR_COMMAND_SYNTAX_ERROR;

	uint32_t addr, value;
	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], addr);
	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[1], value);

	struct flash_bank *bank = get_flash_bank_by_num_noprobe(0);
	if (!bank)
		return ERROR_FAIL;

	struct pic32cx_priv *p = bank->driver_priv;
	const struct pic32cx_dev *dev = (p && p->dev) ? p->dev : &device_table[0];

	target_write_u32(target, dev->nvmdata, value);
	return pic32cx_issue_nvmcmd(dev, target, addr, NVMCON_OP_WORD_PROG, 0);
}

/* manual full-chip erase command: pic32cx_bz erase_chip <banknum?> */
COMMAND_HANDLER(pic32cx_handle_erase_chip_command)
{
	struct target *target = get_current_target(CMD_CTX);
	if (!target)
		return ERROR_FAIL;

	struct flash_bank *bank = get_flash_bank_by_num_noprobe(0);
	if (!bank)
		return ERROR_FAIL;

	return pic32cx_erase_chip(bank);
}

/* command array registration */
static const struct command_registration pic32cx_exec_command_handlers[] = {
	{
		.name = "erase_page",
		.handler = pic32cx_handle_erase_page_command,
		.mode = COMMAND_EXEC,
		.usage = "<address>",
		.help = "Erase a flash page at the given address",
	},
	{
		.name = "write_word",
		.handler = pic32cx_handle_write_word_command,
		.mode = COMMAND_EXEC,
		.usage = "<address> <32bit_hex_value>",
		.help = "Write a 32-bit word to flash at the given address",
	},
	{
		.name = "dsu_reset_deassert",
		.handler = pic32cx_handle_dsu_reset_deassert,
		.mode = COMMAND_EXEC,
		.usage = "",
		.help = "Device-specific DSU reset deassert sequence",
	},
	{
		.name = "erase_chip",
		.handler = pic32cx_handle_erase_chip_command,
		.mode = COMMAND_EXEC,
		.usage = "",
		.help = "Perform a full chip erase using PBC",
	},
	COMMAND_REGISTRATION_DONE
};

static const struct command_registration pic32cx_command_handlers[] = {
	{
		.name = "pic32cx_bz",
		.mode = COMMAND_ANY,
		.help = "pic32cx_bz flash command group",
		.usage = "",
		.chain = pic32cx_exec_command_handlers,
	},
	COMMAND_REGISTRATION_DONE
};

const struct flash_driver pic32cx_bz_flash = {
	.name = "pic32cx_bz",
	.commands = pic32cx_command_handlers,
	.flash_bank_command  = pic32cx_flash_bank_command,
	.erase = pic32cx_erase,
	.protect = NULL,
	.write = pic32cx_write,
	.read = default_flash_read,
	.probe = pic32cx_probe,
	.auto_probe = pic32cx_probe,
	.erase_check = default_flash_blank_check,
	.protect_check = NULL,
	.free_driver_priv = default_flash_free_driver_priv,
};