// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Grandstream Networks, Inc
 *
 * Authors:
 *	Carl <xjxia@grandstream.cn>
 */

#ifndef __UBOOT__
#include <linux/device.h>
#include <linux/kernel.h>
#endif
#include <linux/mtd/spinand.h>


#define SPINAND_MFR_FORESEE		0xCD
#define FORESEE_STATUS_ECC_NO_BITFLIPS	    (0 << 4)
#define FORESEE_STATUS_ECC_1_BITFLIPS	    (1 << 4)
#define FORESEE_ECC_UNCOR_ERROR10           (2 << 4)
#define FORESEE_ECC_UNCOR_ERROR11           (3 << 4)
#define STATUS_ECC_MASK		GENMASK(5, 4)


static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static int fsxxndxxg_ooblayout_ecc(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	/* ECC is not user accessible */
	region->offset = 0;
	region->length = 0;

	return 0;
}

static int fsxxndxxg_ooblayout_free(struct mtd_info *mtd, int section,
				    struct mtd_oob_region *region)
{
	printf("Debug: Foresee layout: ");
	if (section > 3)
		return -ERANGE;

	/*
	 * No ECC data is stored in the accessible OOB so the full 16 bytes
	 * of each spare region is available to the user. Apparently also
	 * covered by the internal ECC.
	 */
	if (section) {
		region->offset = 16 * section;
		region->length = 16;
	} else {
		/* First byte in spare0 area is used for bad block marker */
		region->offset = 2;
		region->length = 14;
	}

	return 0;
}

static const struct mtd_ooblayout_ops fsxxndxxg_ooblayout = {
	.ecc = fsxxndxxg_ooblayout_ecc,
	.rfree = fsxxndxxg_ooblayout_free,
};

static int fsxxndxxg_ecc_get_status(struct spinand_device *spinand, u8 status)
{
	switch (status & STATUS_ECC_MASK) {
	case FORESEE_STATUS_ECC_NO_BITFLIPS:
		return 0;

	case FORESEE_STATUS_ECC_1_BITFLIPS:
		return 1;

	case FORESEE_ECC_UNCOR_ERROR10:
	case FORESEE_ECC_UNCOR_ERROR11:	
		return -EBADMSG;
	
	default:
		break;
	}

	return -EINVAL;
}

static const struct spinand_info foresee_spinand_table[] = {
	SPINAND_INFO("F35SQA512M",
		     0x70,
		     NAND_MEMORG(1, 2048, 64, 64, 512, 10, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&fsxxndxxg_ooblayout, fsxxndxxg_ecc_get_status)),
	SPINAND_INFO("F35SQA001G",
		     0x71,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&fsxxndxxg_ooblayout, fsxxndxxg_ecc_get_status)),
	SPINAND_INFO("F35SQA002G",
		     0x72,
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&fsxxndxxg_ooblayout, fsxxndxxg_ecc_get_status)),
	SPINAND_INFO("F35UQA512M",
		     0x60,
		     NAND_MEMORG(1, 2048, 64, 64, 512, 10, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&fsxxndxxg_ooblayout, fsxxndxxg_ecc_get_status)),
	SPINAND_INFO("F35UQA001G",
		     0x61,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&fsxxndxxg_ooblayout, fsxxndxxg_ecc_get_status)),
	SPINAND_INFO("F35UQA002G",
		     0x62,
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&fsxxndxxg_ooblayout, fsxxndxxg_ecc_get_status)),
};

static int foresee_spinand_detect(struct spinand_device *spinand)
{
	u8 *id = spinand->id.data;
	int ret;
	/*
	 * For Foresee NANDs, There is an address byte needed to shift in before IDs
	 * are read out, so the first byte in raw_id is dummy.
	 */
	if (id[1] != SPINAND_MFR_FORESEE)
		return 0;

	ret = spinand_match_and_init(spinand, foresee_spinand_table,
				     ARRAY_SIZE(foresee_spinand_table),
				     id[2]);
	printf("Mfg. is 0x%02X, device is 0x%02X match and init status 0x%02X\n", id[1], id[2], ret);
	if (ret)
		return ret;

	return 1;
}

static const struct spinand_manufacturer_ops foresee_spinand_manuf_ops = {
	.detect = foresee_spinand_detect,
};

const struct spinand_manufacturer foresee_spinand_manufacturer = {
	.id = SPINAND_MFR_FORESEE,
	.name = "foresee",
	.ops = &foresee_spinand_manuf_ops,
};
