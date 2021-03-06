#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "bt_wrapper.h"

uint32_t sys_time = 0;
uint32_t last_sys_time = 0;

#define CONF_BSP_TICKS_PER_SEC 100


#define HW_DEBUG	printf

#define hw_memset 	memset
#define hw_memcpy	memcpy
#define hw_memcmp	memcmp
#define hw_strlen	strlen
#define hw_strcmp strcmp
#define hw_strncmp strncmp
#define hw_sprintf sprintf

#define HW_ERR_OK 0
#define HW_ERR_SHELL_NO_CMD 1


#define BT_START_CMD "BT_START"
#define BT_START_DES "Start bluetooth stack"
#define BT_STOP_CMD "BT_STOP"
#define BT_STOP_DES "Stop blueooth stack"
#define BT_INQUIRY_CMD "BT_INQUIRY"
#define BT_INQUIRY_DES "Inquiry device"
#define BT_CANCEL_INQUIRY_CMD "BT_CANCEL_INQUIRY"
#define BT_CANCEL_INQUIRY_DES "Cancel inquiry device"
#define BT_PERIOID_INQUIRY_CMD "BT_PERIOID_INQUIRY"
#define BT_PERIOID_INQUIRY_DES "Perioid inquiry device"
#define BT_CANCEL_PERIOID_INQUIRY_CMD "BT_CANCEL_PERIOID_INQUIRY"
#define BT_CANCEL_PERIOID_INQUIRY_DES "Cancel perioid inquiry device"
#define BT_LE_INQUIRY_CMD "BT_LE_INQUIRY"
#define BT_LE_INQUIRY_DES "BLE Inquiry device"
#define BT_LE_INQUIRY_CANCEL_CMD "BT_LE_INQUIRY_STOP"
#define BT_LE_INQUIRY_CANCEL_DES "BLE cancel Inquiry device"
#define BT_SPP_CON_CMD "SPP_CON"
#define BT_SPP_CON_DES "Connect spp profile"
#define BT_SPP_SEND_CMD "SPP_SEND"
#define BT_SPP_SEND_DES "Spp sned data"
#define BT_SPP_DISCON_CMD "SPP_DISCON"
#define BT_SPP_DISCON_DES "Disconnect spp profile"
#define BT_HFP_CON_CMD "HFP_CON"
#define BT_HFP_CON_DES "Connect hfp profile"
#define BT_HFP_DISCON_CMD "HFP_DISCON"
#define BT_HFP_DISCON_DES "Disconnect hfp profile"
#define BT_HFP_SCO_CON_CMD "HFP_SCO_CON"
#define BT_HFP_SCO_CON_DES "Connect hfp sco"
#define BT_HFP_SCO_DISCON_CMD "HFP_SCO_DISCON"
#define BT_HFP_SCO_DISCON_DES "Disconnect hfp sco"
#define BT_HFP_ANSWER_CMD "HFP_ANSWER"
#define BT_HFP_ANSWER_DES "Answer the incoming call"
#define BT_HFP_CALLOUT_PN_CMD "HFP_CALLOUT_PN"
#define BT_HFP_CALLOUT_PN_DES "Call out phone number(10086)"
#define BT_HFP_CALLOUT_MEM_CMD "HFP_CALLOUT_MEM"
#define BT_HFP_CALLOUT_MEM_DES "Call out phone number with memory 1"
#define BT_HFP_CALLOUT_LN_CMD "HFP_CALLOUT_LC"
#define BT_HFP_CALLOUT_LN_DES "Call out last number"




typedef struct
{
    uint8_t *cmd;
    uint8_t *description;
} cmd_desctiption_t;

cmd_desctiption_t cmd_usage[] =
{
    {(uint8_t *)BT_START_CMD,(uint8_t *)BT_START_DES},
    {(uint8_t *)BT_STOP_CMD,(uint8_t *)BT_STOP_DES},
    {(uint8_t *)BT_INQUIRY_CMD,(uint8_t *)BT_INQUIRY_CMD},
    {(uint8_t *)BT_CANCEL_INQUIRY_CMD,(uint8_t *)BT_CANCEL_INQUIRY_DES},
    {(uint8_t *)BT_PERIOID_INQUIRY_CMD,(uint8_t *)BT_PERIOID_INQUIRY_DES},
    {(uint8_t *)BT_CANCEL_PERIOID_INQUIRY_CMD,(uint8_t *)BT_CANCEL_PERIOID_INQUIRY_DES},
    {(uint8_t *)BT_LE_INQUIRY_CMD,(uint8_t *)BT_LE_INQUIRY_DES},
    {(uint8_t *)BT_LE_INQUIRY_CANCEL_CMD,(uint8_t *)BT_LE_INQUIRY_CANCEL_DES},
    {(uint8_t *)BT_SPP_CON_CMD,(uint8_t *)BT_SPP_CON_DES},
    {(uint8_t *)BT_SPP_SEND_CMD,(uint8_t *)BT_SPP_SEND_DES},
    {(uint8_t *)BT_SPP_DISCON_CMD,(uint8_t *)BT_SPP_DISCON_DES},
    {(uint8_t *)BT_HFP_CON_CMD,(uint8_t *)BT_HFP_CON_DES},
    {(uint8_t *)BT_HFP_DISCON_CMD,(uint8_t *)BT_HFP_DISCON_DES},
    {(uint8_t *)BT_HFP_SCO_CON_CMD,(uint8_t *)BT_HFP_SCO_CON_DES},
    {(uint8_t *)BT_HFP_SCO_DISCON_CMD,(uint8_t *)BT_HFP_SCO_DISCON_DES},
    {(uint8_t *)BT_HFP_ANSWER_CMD,(uint8_t *)BT_HFP_ANSWER_DES},
    {(uint8_t *)BT_HFP_CALLOUT_PN_CMD,(uint8_t *)BT_HFP_CALLOUT_PN_DES},
    {(uint8_t *)BT_HFP_CALLOUT_MEM_CMD,(uint8_t *)BT_HFP_CALLOUT_MEM_DES},
    {(uint8_t *)BT_HFP_CALLOUT_LN_CMD,(uint8_t *)BT_HFP_CALLOUT_LN_DES},
};


void show_usage()
{
    uint32_t index = 0;
    for(index = 0; index < sizeof(cmd_usage)/sizeof(cmd_desctiption_t); index++)
    {
        HW_DEBUG("CMD(%s) -> DESCRIPTION(%s)\n",cmd_usage[index].cmd,cmd_usage[index].description);
    }
}

struct bd_addr_t connect_addr;


void bt_app_init_result(uint8_t status,uint16_t profile_mask)
{
    uint8_t profile_mask_buf[8] = {0};
    printf("bt_app_init_result(%d) profile_mask(0x%x)\n",status,profile_mask);

}

void bt_app_inquiry_status(uint8_t status)
{
    printf("bt_inquiry_status %d\n",status);
}

void bt_app_inquiry_result(struct bd_addr_t *address,uint8_t dev_type,uint8_t *name)
{
    printf("-----------inquiry result ----------\n");
    printf("address:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\n",address->addr[0],address->addr[1],address->addr[2],\
           address->addr[3],address->addr[4],address->addr[5]);
    printf("type %d\n",dev_type);
    printf("name %s\n",name);
    printf("----------------------- ----------\n");
}

static bt_app_common_cb_t bt_app_common_cb =
{
    bt_app_init_result,
    bt_app_inquiry_status,
    bt_app_inquiry_result,
};

void bt_app_hfp_connect(struct bd_addr_t *remote_addr,uint8_t status)
{
    printf("bt_app_hfp_connect status %d address:\n",status);
    bt_hex_dump(remote_addr->addr,6);
    connect_addr.addr[5] = remote_addr->addr[5];
    connect_addr.addr[4] = remote_addr->addr[4];
    connect_addr.addr[3] = remote_addr->addr[3];
    connect_addr.addr[2] = remote_addr->addr[2];
    connect_addr.addr[1] = remote_addr->addr[1];
    connect_addr.addr[0] = remote_addr->addr[0];
}

void bt_app_hfp_disconnect(struct bd_addr_t *remote_addr,uint8_t status)
{
    printf("bt_app_hfp_disconnect status %d address:\n",status);
    bt_hex_dump(remote_addr->addr,6);

}


void bt_app_hfp_signal_strength_ind(struct bd_addr_t *remote_addr,uint8_t value)
{
	printf("bt_app_hfp_signal_strength_ind value %d address:\n",value);
    bt_hex_dump(remote_addr->addr,6);

}

void bt_app_hfp_roam_status_ind(struct bd_addr_t *remote_addr,uint8_t value)
{
	printf("bt_hfp_roam_status_ind value %d address:\n",value);
    bt_hex_dump(remote_addr->addr,6);

}

void bt_app_hfp_batt_level_ind(struct bd_addr_t *remote_addr,uint8_t value)
{
	printf("bt_hfp_batt_level_ind value %d address:\n",value);
    bt_hex_dump(remote_addr->addr,6);

}

void bt_app_hfp_operator(struct bd_addr_t *remote_addr,uint8_t *operator)
{
	printf("bt_app_hfp_operator operator %s address:\n",operator);
    bt_hex_dump(remote_addr->addr,6);

}

void bt_app_hfp_call_status(struct bd_addr_t *remote_addr,uint8_t value)
{
	printf("bt_app_hfp_call_status value %d address:\n",value);
    bt_hex_dump(remote_addr->addr,6);
}

void bt_app_hfp_call_setup(struct bd_addr_t *remote_addr,uint8_t value)
{
	printf("bt_app_hfp_call_setup value %d address:\n",value);
    bt_hex_dump(remote_addr->addr,6);

}


static bt_app_hfp_cb_t bt_app_hfp_cb =
{
    bt_app_hfp_connect,
	bt_app_hfp_disconnect,
    bt_app_hfp_signal_strength_ind,
    bt_app_hfp_roam_status_ind,
    bt_app_hfp_batt_level_ind,
    bt_app_hfp_operator,
    bt_app_hfp_call_status,
    bt_app_hfp_call_setup,
};


void bt_app_spp_connect(struct bd_addr_t *remote_addr,uint8_t status)
{
    printf("bt_app_spp_connect status %d address:\n",status);
    bt_hex_dump(remote_addr->addr,6);
    connect_addr.addr[5] = remote_addr->addr[5];
    connect_addr.addr[4] = remote_addr->addr[4];
    connect_addr.addr[3] = remote_addr->addr[3];
    connect_addr.addr[2] = remote_addr->addr[2];
    connect_addr.addr[1] = remote_addr->addr[1];
    connect_addr.addr[0] = remote_addr->addr[0];
}

void bt_app_spp_disconnect(struct bd_addr_t *remote_addr,uint8_t status)
{
    printf("bt_app_spp_disconnect status %d address:\n",status);
    bt_hex_dump(remote_addr->addr,6);
    memset(&connect_addr,0,sizeof(connect_addr));
}

void bt_app_spp_recv_data(struct bd_addr_t *remote_addr,uint8_t *data,uint16_t data_len)
{
    printf("bt_app_spp_recv_data len %d address:\n",data_len);
    bt_hex_dump(remote_addr->addr,6);
    printf("data is :");
    bt_hex_dump(data,data_len);
}

static bt_app_spp_cb_t bt_app_spp_cb =
{
    bt_app_spp_connect,
    bt_app_spp_disconnect,
    bt_app_spp_recv_data,
};


static bt_app_cb_t bt_app_cb =
{
    &bt_app_common_cb,
    &bt_app_spp_cb,
    &bt_app_hfp_cb,
};


#define LINUX_SPP_SEND_STRING "Hello,I am SPP in Linux"
uint8_t shell_parse(uint8_t *shell_string)
{
    if(hw_strcmp(BT_START_CMD,(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate bt start\n");
        bt_start(&bt_app_cb);
        return HW_ERR_OK;
    }

    if(hw_strcmp(BT_STOP_CMD,(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate bt stop\n");
        bt_stop();
        return HW_ERR_OK;
    }

    if(hw_strcmp(BT_INQUIRY_CMD,(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate bt inquiry\n");
        bt_start_inquiry(0x30,HCI_INQUIRY_MAX_DEV);
        return HW_ERR_OK;
    }

    if(hw_strcmp(BT_CANCEL_INQUIRY_CMD,(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate bt cancel inquiry\n");
        bt_stop_inquiry();
        return 0;
    }

#if PROFILE_SPP_ENABLE > 0
    if(hw_strncmp(BT_SPP_SEND_CMD,(const char*)shell_string,hw_strlen(BT_SPP_SEND_CMD)) == 0)
    {
        HW_DEBUG("SHELL:operate bt spp send \n");
        spp_send_data(&connect_addr,(uint8_t *)LINUX_SPP_SEND_STRING,hw_strlen(LINUX_SPP_SEND_STRING));
        return HW_ERR_OK;
    }

    if(hw_strncmp(BT_SPP_CON_CMD,(const char*)shell_string,hw_strlen(BT_SPP_CON_CMD)) == 0)
    {
        HW_DEBUG("SHELL:operate spp CON\n");

        spp_connect(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strncmp(BT_SPP_DISCON_CMD,(const char*)shell_string,hw_strlen(BT_SPP_DISCON_CMD)) == 0)
    {
        HW_DEBUG("SHELL:operate spp DISCON\n");

        spp_disconnect(&connect_addr);
        return HW_ERR_OK;
    }
#endif

#if PROFILE_PBAP_ENABLE > 0
    if(hw_strcmp("PBAP_CON",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP CON\n");

        pbap_client_connect(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_DISCON",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP DISCON\n");

        pbap_client_disconnect(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_LP",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LPB\n");

        pbap_client_download_phonebook(&connect_addr,PB_LOCAL_REPOSITORY,PB_PHONEBOOK_TYPE);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_LI",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LI\n");

        pbap_client_download_phonebook(&connect_addr,PB_LOCAL_REPOSITORY,PB_INCOMING_BOOK_TYPE);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_LO",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LI\n");

        pbap_client_download_phonebook(&connect_addr,PB_LOCAL_REPOSITORY,PB_OUTGOING_BOOK_TYPE);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_LM",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LI\n");

        pbap_client_download_phonebook(&connect_addr,PB_LOCAL_REPOSITORY,PB_MISSING_BOOK_TYPE);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_LC",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LI\n");

        pbap_client_download_phonebook(&connect_addr,PB_LOCAL_REPOSITORY,PB_COMBINE_BOOK_TYPE);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_LPC",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LPB\n");

        pbap_client_query_phonebook_size(&connect_addr,PB_LOCAL_REPOSITORY,PB_PHONEBOOK_TYPE);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_LIC",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LI\n");

        pbap_client_query_phonebook_size(&connect_addr,PB_LOCAL_REPOSITORY,PB_INCOMING_BOOK_TYPE);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_LOC",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LI\n");

        pbap_client_query_phonebook_size(&connect_addr,PB_LOCAL_REPOSITORY,PB_OUTGOING_BOOK_TYPE);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_LMC",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LI\n");

        pbap_client_query_phonebook_size(&connect_addr,PB_LOCAL_REPOSITORY,PB_MISSING_BOOK_TYPE);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_LCC",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LI\n");

        pbap_client_query_phonebook_size(&connect_addr,PB_LOCAL_REPOSITORY,PB_COMBINE_BOOK_TYPE);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_SP",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LPB\n");

        pbap_client_set_path(&connect_addr,PB_LOCAL_REPOSITORY,PB_PHONEBOOK_TYPE);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_SPL",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LPB\n");

        pbap_client_download_vcard_list(&connect_addr,PB_LOCAL_REPOSITORY,PB_PHONEBOOK_TYPE);
        return HW_ERR_OK;
    }

    if(hw_strcmp("PBAP_DVE",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate PBAP PBAP_LPB\n");

        pbap_client_download_vcard_entry(&connect_addr,PB_LOCAL_REPOSITORY,PB_PHONEBOOK_TYPE,1);
        return HW_ERR_OK;
    }
#endif


#if PROFILE_HFP_ENABLE > 0
    if(hw_strcmp("HFP_CON",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate HFP CON\n");

        hfp_hf_connect(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_DISCON",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate HFP DISCON\n");

        hfp_hf_disconnect(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_SCO_CON",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate HFP SCO CON\n");

        hfp_hf_audio_connect(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_SCO_DISCON",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate HFP SCO DISCON\n");

        hfp_hf_audio_disconnect(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_ANSWER",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate HFP ANSWER INCOMING CALL\n");

        hfp_hf_answer_incoming_call(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_CALLOUT_PN",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate call out number\n");
        hfp_hf_callout_with_phone_number(&connect_addr,"10086");
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_CALLOUT_MEM",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate bt stop\n");
        hfp_hf_callout_with_memory(&connect_addr,1);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_CALLOUT_LC",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate bt stop\n");
        hfp_hf_callout_with_last_number(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_WN",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate call wait enable\n");
        hfp_hf_set_call_waiting_notification(&connect_addr,1);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_WD",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate call wait disenable\n");
        hfp_hf_set_call_waiting_notification(&connect_addr,0);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_CLIE",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate CLI enable\n");
        hfp_hf_set_call_line_identification_notification(&connect_addr,1);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_CLID",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate CLI disable\n");
        hfp_hf_set_call_line_identification_notification(&connect_addr,0);
        return HW_ERR_OK;
    }


    if(hw_strcmp("HFP_NRECD",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate CLI disable\n");
        hfp_hf_disable_ag_nrec(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_VGE",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate CLI enable\n");
        hfp_hf_set_voice_recognition(&connect_addr,1);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_VGD",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate CLI disable\n");
        hfp_hf_set_voice_recognition(&connect_addr,0);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_GPN",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate get phone number via voice tag\n");
        hfp_hf_get_phone_number_via_voice_tag(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_DTMF",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate get phone number via voice tag\n");
        hfp_hf_transmit_dtmf(&connect_addr,1);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_VGM",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate VGM\n");
        hfp_hf_set_mic_volume(&connect_addr,1);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_VGS",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate VGM\n");
        hfp_hf_set_spk_volume(&connect_addr,1);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_LPN",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate local number\n");
        hfp_hf_get_local_phone_number(&connect_addr);
        return HW_ERR_OK;
    }


    if(hw_strcmp("HFP_CLCC",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate bt stop\n");
        hfp_hf_query_call_list(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_CALLEND",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate bt stop\n");
        hfp_hf_hangup(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_NET_N",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate operate hfp get operate\n");
        bt_hfp_hf_get_operator(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_I",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate HFP_I\n");
        hfp_hf_set_indicator_enable_value(&connect_addr,"call",0);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_CGMI",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate HFP_I\n");
        hfp_hf_get_manufacturer_id(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_CGMM",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate HFP_I\n");
        hfp_hf_get_model_id(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_CGMR",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate HFP_I\n");
        hfp_hf_get_revision_id(&connect_addr);
        return HW_ERR_OK;
    }

    if(hw_strcmp("HFP_PID",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:operate HFP_I\n");
        hfp_hf_get_pid(&connect_addr);
        return HW_ERR_OK;
    }
#endif


#if PROFILE_AVRCP_ENABLE > 0
    if(hw_strcmp("AVRCP_GET_CAP1",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:AVRCP_GET_CAP1\n");
        avrcp_controller_get_supported_company_ids();
        return HW_ERR_OK;
    }

    if(hw_strcmp("AVRCP_GET_CAP2",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:AVRCP_GET_CAP2\n");
        avrcp_controller_get_supported_events();
        return HW_ERR_OK;
    }

    if(hw_strcmp("AVRCP_REG_TRACK_C",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:AVRCP_REG_TRACK_C\n");
        avrcp_controller_enable_notification(AVRCP_NOTIFICATION_EVENT_TRACK_CHANGED);
        return HW_ERR_OK;
    }

    if(hw_strcmp("AVRCP_GET_INFO",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:AVRCP_REG_TRACK_C\n");
        uint8_t media_info[7] = {1,2,3,4,5,6,7};
        avrcp_controller_get_element_attributes(media_info,sizeof(media_info));
        return HW_ERR_OK;
    }

    if(hw_strcmp("AVRCP_PLAY",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:AVRCP PLAY\n");
        avrcp_controller_play();
        return HW_ERR_OK;
    }

    if(hw_strcmp("AVRCP_PAUSE",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:AVRCP pause\n");
        avrcp_controller_pause();
        return HW_ERR_OK;
    }

    if(hw_strcmp("AVRCP_FORWARD",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:AVRCP forward\n");
        avrcp_controller_forward();
        return HW_ERR_OK;
    }

    if(hw_strcmp("AVRCP_BACKWARD",(const char*)shell_string) == 0)
    {
        HW_DEBUG("SHELL:AVRCP backward\n");
        avrcp_controller_backward();
        return HW_ERR_OK;
    }
#endif


    show_usage();
    return HW_ERR_SHELL_NO_CMD;
}


#define UART1_MAX_REV	1024
uint8_t uart_rev_buffer[UART1_MAX_REV];
uint8_t uart_rev_len;

void *timer_thread(void *data)
{
    printf("timer_thread runing...\n");
    while(1)
    {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1000/CONF_BSP_TICKS_PER_SEC * 1000;

        select(0, NULL, NULL, NULL, &tv);
        sys_time += 1000/CONF_BSP_TICKS_PER_SEC;

        utimer_polling();
    }
}

#define STDIN_BUF_SIZE 128
uint8_t stdin_buf[STDIN_BUF_SIZE] = {0};
uint8_t stdin_recv_len = 0;

void *stdin_process_thread(void *data)
{

    while(1)
    {
        fd_set read_fd;
        int result = 0;

        /* monitor uart rx */
        FD_ZERO (&read_fd);
        FD_SET (0, &read_fd);

        if ((result = select (1, &read_fd, NULL, NULL, NULL)) == -1)
        {
            printf("ERROR:file[%s],function[%s],line[%d] select fail\n",__FILE__,__FUNCTION__,__LINE__);
        }

        if(result > 0 && FD_ISSET ( 0, &read_fd))
        {

            memset(stdin_buf,0,STDIN_BUF_SIZE);
            int read_result = read(0,stdin_buf,STDIN_BUF_SIZE);
            stdin_recv_len = read_result;
            //printf("--------1---------len %d\n",read_result);
            //bt_hex_dump(stdin_buf,read_result);
            //printf("--------2----------\n");

        }
    }
}


void stdin_process_init()
{

    pthread_t thread_stdin_id;
    pthread_create(&thread_stdin_id, NULL, stdin_process_thread, NULL);
}
void stdin_process_run()
{
    if(stdin_recv_len != 0)
    {
        shell_parse(stdin_buf);
        stdin_recv_len = 0;
    }
}



void board_init()
{
    pthread_t thread_timer_id;

    last_sys_time = sys_time;
    utimer_init();

    bt_start(&bt_app_cb);
    pthread_create(&thread_timer_id, NULL, timer_thread, NULL);


}



extern struct phybusif_cb uart_if;
int main()
{
    stdin_process_init();
    board_init();

    while(1)
    {

        stdin_process_run();
        if(phybusif_input(&uart_if) != BT_ERR_OK)
            usleep(1000);

        if(sys_time - last_sys_time > 1000)
        {
            //printf("bt stack running\n");
            last_sys_time = sys_time;
            l2cap_tmr();
            rfcomm_tmr();
        }


    }
}
