#include "ws.h"

extern int num_conns;

void send_ws_close(client *conn) {
  unsigned char frame[2] = {0}; // 2 bytes for header, max 125 bytes for payload

  frame[0] = 0x88; // fin = 1, opcode = 8 for close

  size_t nsent = send(conn->fd, frame, 2, 0);
  printf("data sent: ");
  for (int i = 0; i < nsent; i++) {
    printf("%02x ", frame[i]);
  }
  printf("\n");
  printf("sent %zu bytes\n", nsent);
}

void send_ws_message(client *conn, char *message, int n) {
  if (n > 125) {
    fprintf(stderr, "Error sending ws data: data size %d too large\n", n);
    return;
  }

  unsigned char frame[125] = {
      0}; // 2 bytes for header, max 125 bytes for payload

  frame[0] = 0x81;     // fin = 1, opcode = 1 for text
  frame[1] = n & 0x7f; // length of data (mask bit always 0) 0111 1111

  memcpy(frame + 2, message, n + 2);

  size_t nsent = write(conn->fd, frame, n + 2);
  printf("data sent: ");
  for (int i = 0; i < nsent; i++) {
    printf("%02x ", frame[i]);
  }
  printf("\n");
  printf("sent %zu bytes\n", nsent);
}

int receive_ws_data(ws_frame* dst_frame, client *conn) {
  int nread = read(conn->fd, dst_frame->buf, sizeof(dst_frame->buf));
  if (nread == -1) {
    fprintf(stderr, "Error reading message\n");
    return 0;
  }

  dst_frame->msg_len = dst_frame->buf[1] & 0x7f;
  memcpy(dst_frame->mask, dst_frame->buf + 2, 4); // mask at most 4 bytes
  memcpy(dst_frame->message, dst_frame->buf + 6, MAX_WS_MSG_SIZE);
  printf("data frame Hex: ");
  for (int i = 0; i < nread; i++) {
    printf("%02x ", dst_frame->buf[i]);
  }
  printf("\n");

  dst_frame->opcode = dst_frame->buf[0] & 0x0F;

  return dst_frame->opcode;
}
