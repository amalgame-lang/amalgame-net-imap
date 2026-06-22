# amalgame-net-imap

An **IMAP4rev1 server** for the native Amalgame mail server (Phase 4). It
lets a mail client (Thunderbird / K-9 / Apple Mail) connect and **read**
the messages SMTP delivered into an
[`amalgame-net-mail-store`](https://github.com/amalgame-lang/amalgame-net-mail-store)
MailStore. Pairs with
[`amalgame-net-smtp-server`](https://github.com/amalgame-lang/amalgame-net-smtp-server)
(receive) — together they make `family.neitsab.fr` usable as a real mail
account. See
[`native-mail-server.md`](https://github.com/amalgame-lang/Amalgame/blob/main/docs/proposals/native-mail-server.md).

## Two layers

| Layer | Status |
|---|---|
| **`ImapSession`** — the IMAP command state machine (pure logic, socket/TLS-free, unit-tested 14/14) | ✅ v0.1.0 |
| **`ImapServer`** — the TcpServer accept loop + STARTTLS upgrade (transport, loopback smoke-tested) | ✅ v0.1.0 |

`ImapSession` is the heart: feed it one tagged command line and it
returns the response (untagged data lines + the tagged completion). It
owns no socket and no TLS, so it is fully unit-testable with strings.

```amalgame
import Amalgame.Net.Imap
import Amalgame.Net.Mail

let store: MailStore = MailStore.Open("/var/mail/alice")
let srv:   ImapServer = new ImapServer(store, "mail.example.com")
srv.AddPassword("alice", "s3cret")                 // or AddUserHash(name, scryptHash)
let s2: ImapServer = srv.WithCert("/etc/ssl/cert.pem", "/etc/ssl/key.pem")
s2.Serve(143)                                       // blocking; one connection at a time (v0.1)
```

## Command coverage (v0.1.0)

- **CAPABILITY** — advertises `STARTTLS` + `LOGINDISABLED` before TLS,
  `IMAP4rev1` after.
- **STARTTLS** — the transport upgrades the socket via `amalgame-tls`,
  then `SetTlsActive(true)`.
- **LOGIN** — verified against **scrypt** hashes via `amalgame-crypto`
  `Password.Verify` (the same hashes `amalgame-auth` stores → one unified
  account). Refused before TLS (`[PRIVACYREQUIRED]`).
- **LIST / LSUB** — mailboxes from the store (always advertises `INBOX`).
- **SELECT / EXAMINE** — `EXISTS`, `RECENT`, `UIDVALIDITY`, `UIDNEXT`,
  `FLAGS`, `PERMANENTFLAGS`.
- **FETCH / UID FETCH** — `FLAGS`, `UID`, `RFC822.SIZE`, and the message
  body as `BODY[]` / `RFC822` / `BODY.PEEK[]` (literal-encoded), over a
  sequence/UID set (`1`, `2:4`, `1,3`, `2:*`, `*`).
- **STORE / UID STORE** — `FLAGS` / `+FLAGS` / `-FLAGS` (`.SILENT`).
- **SEARCH / UID SEARCH** — `ALL`, `SEEN`, `UNSEEN`, `DELETED`, `FLAGGED`.
- **EXPUNGE / CLOSE** — removes `\Deleted` messages (high→low sequence).
- **NOOP / CHECK / LOGOUT**.

Verified end-to-end by `tests/smoke_test.sh`: a real Python `imaplib`
client does STARTTLS → LOGIN → SELECT → SEARCH → FETCH and reads the
message back.

## Out of scope (v0.1.0)

- `ENVELOPE` / `BODYSTRUCTURE` / partial `BODY[<section>]<n.m>` — a client
  can fetch the whole message (`BODY[]`) and parse it with
  `amalgame-formats-mime`.
- `IDLE`, `APPEND`, `COPY`, multi-mailbox hierarchies beyond the store,
  `AUTHENTICATE` SASL mechanisms (LOGIN command only), a worker pool
  (v0.1 serves one connection at a time), implicit-TLS on `:993`.

## Dependencies

- [`amalgame-net-mail-store`](https://github.com/amalgame-lang/amalgame-net-mail-store) `>=0.1.0`
- [`amalgame-crypto`](https://github.com/amalgame-lang/amalgame-crypto) `>=0.6.0`
- [`amalgame-tls`](https://github.com/amalgame-lang/amalgame-tls) `>=0.3.5`

## Tests

```sh
# siblings net-mail-store, io-filesystem, database-sqlite, crypto, tls alongside
bash tests/run_tests.sh  /path/to/amc     # ImapSession unit tests (14/14)
bash tests/smoke_test.sh /path/to/amc     # loopback IMAP+STARTTLS via Python imaplib
```

## License

Apache-2.0 — see `LICENSE` and `NOTICE.md`.
