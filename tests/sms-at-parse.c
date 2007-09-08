/* Test for decoding SMS on Nokia 6110 driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../common/protocol/protocol.h" /* Needed for GSM_Protocol_Message */
#include "../common/gsmstate.h" /* Needed for state machine internals */
#include "../common/gsmphones.h" /* Phone data */

GSM_StateMachine *s;
GSM_Protocol_Message msg;
GSM_Error error;
GSM_MultiSMSMessage sms;
GSM_MultiPartSMSInfo	SMSInfo;

/* This is not part of API! */
extern GSM_Error ATGEN_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s);

#define BUFFER_SIZE 16384

int main(int argc, char **argv)
{
	GSM_Debug_Info *di;
	GSM_Phone_ATGENData *Priv;
	GSM_Phone_Data *Data;
	unsigned char buffer[BUFFER_SIZE];
	FILE *f;
	size_t len;

	/* Check parameters */
	if (argc != 2) {
		printf("Not enough parameters!\nUsage: sms-at-parse comm.dump\n");
		return 1;
	}

	/* Open file */
	f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("Could not open %s\n", argv[1]);
		return 1;
	}

	/* Read data */
	len = fread(buffer, 1, sizeof(buffer) - 1, f);
	if (!feof(f)) {
		printf("Could not read whole file %s\n", argv[1]);
		return 1;
	}
	buffer[len] = 0;

	/* Configure state machine */
	di = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, di);
	GSM_SetDebugLevel("textall", di);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL) {
		printf("Could not allocate state machine!\n");
		return 1;
	}

	/* Init message */
	Data = &s->Phone.Data;
	Data->ModelInfo = GetModelData(NULL, "unknown", NULL);
	Priv = &s->Phone.Data.Priv.ATGEN;
	Priv->ReplyState = AT_Reply_OK;
	Priv->SMSMode = SMS_AT_PDU;
	msg.Type = 0;
	msg.Length = len;
	msg.Buffer = buffer;
	SplitLines(msg.Buffer, msg.Length, &Priv->Lines, "\x0D\x0A", 2, true);

	s->Phone.Data.GetSMSMessage = &sms;

	/* Parse it */
	error = ATGEN_ReplyGetSMSMessage(msg, s);

	/* Free state machine */
	GSM_FreeStateMachine(s);

	printf("%s\n", GSM_ErrorString(error));

	return (error == ERR_NONE) ? 0 : 1;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
