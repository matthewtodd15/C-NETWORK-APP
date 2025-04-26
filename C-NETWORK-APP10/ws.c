#include "ws.h"
#include "matchmaking.h"

void send_ws_close(client *conn)
{
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

unsigned char* ws_frame_to_bytes(ws_frame* frame, int* len) {
  unsigned char* frame_bytes = malloc(MAX_WS_FRAME_SIZE);

  frame_bytes[0] = 0x80 | frame->opcode;  // fin = 1
  frame_bytes[1] = frame->msg_len & 0x7f; // length of data (mask bit always 0) 0111 1111

  memcpy(frame_bytes + 2, frame->msg, frame->msg_len);
  *len = frame->msg_len + 2;

  return frame_bytes;
}

void send_ws_message(client *conn, char *message, int n)
{
  if (n > MAX_WS_MSG_SIZE) {
    fprintf(stderr, "Error sending ws data: data size %d too large\n", n);
    return;
  }

  unsigned char frame[MAX_WS_MSG_SIZE] = {
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

int receive_ws_data(ws_frame *dst_frame, client *conn)
{
  int nread = read(conn->fd, dst_frame->buf, sizeof(dst_frame->buf));
  if (nread == -1) {
    fprintf(stderr, "Error reading message\n");
    return OP_CLOSE;
  }
  else if (nread == 0) {
    fprintf(stderr, "Client closed the connection\n");
    return OP_CLOSE;
  }

  dst_frame->msg_len = dst_frame->buf[1] & 0x7f;
  memcpy(dst_frame->mask, dst_frame->buf + 2, 4); // mask at most 4 bytes

  printf("data frame Hex: ");
  for (int i = 0; i < nread; i++) {
    printf("%02x ", dst_frame->buf[i]);
  }
  printf("\n");

  // Unmask message from client
  printf("data frame message: ");
  for (int i = 0; i < dst_frame->msg_len && i < MAX_WS_MSG_SIZE; i++) {
    dst_frame->msg[i] = dst_frame->buf[i + 6] ^ dst_frame->mask[i % 4];
    printf("%02x ", dst_frame->msg[i]);
  }
  printf("\n");

  dst_frame->opcode = dst_frame->buf[0] & 0x0F;
  printf("WS Len, Op: %d, %d\n", dst_frame->msg_len, dst_frame->opcode);

    if (strcmp((char*)dst_frame->msg, "ready") == 0) {
    handle_player_ready(conn->fd);
    return OP_TEXT;
  }
return dst_frame->opcode;
}
