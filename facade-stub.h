/*
 * facade-stub.h — runtime header for the imap facade.
 *
 * ImapSession is pure Amalgame; ImapServer's TCP/TLS transport uses the
 * runtime's built-in TcpServer/TcpConn plus amalgame-tls. This file
 * exists only because the manifest's [stdlib].header field is required
 * by amc's PackageRegistry.LoadFrom; the user binary's #include is a
 * no-op.
 */
