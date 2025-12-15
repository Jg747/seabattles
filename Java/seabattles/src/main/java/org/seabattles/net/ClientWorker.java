package org.seabattles.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.util.Optional;
import java.util.UUID;

import org.json.JSONObject;
import org.seabattles.src.Logger;

public class ClientWorker implements Runnable {
	
	// Server thread for client handling
	
	private class ClientObj {
	
		private Socket sock;
		private InputStream in;
		private PrintWriter out;
		private byte[] buffer;
		
		private UUID id;
		
		public ClientObj(Socket sock) {
			this.sock = sock;
			
			try {
				in = sock.getInputStream();
				out = new PrintWriter(sock.getOutputStream());
			} catch (IOException e) {
				e.printStackTrace();
				System.err.println(e);
				destroy();
			}
			
			buffer = new byte[Server.BUF_SIZE];
		}
		
		public void setID(UUID id) {
			this.id = id;
		}
		
		public Optional<JSONObject[]> waitMsg() {
			try {
				int len = in.read(buffer);
				if ((len > 2 && buffer[0] == '{' && buffer[1] == '\"') || len == 2 && buffer[0] == '{' && buffer[1] == '}') {
					String[] msgs = new String(buffer, 0, len).split("\\n");
					JSONObject[] ret = new JSONObject[msgs.length];
					for (int i = 0; i < msgs.length; i++) {
						ret[i] = new JSONObject(msgs[i]);
						write("Received msg: \'" + ret[i] + "\'");
					}
					return Optional.of(ret);
				} else {
					// TODO SPRITE
				}
			} catch (IOException e) {
				e.printStackTrace();
				System.err.println(e);
				destroy();
			}
			return Optional.empty();
		}
		
		public void sendMsg(String msg) {
			write("Sending msg: \'" + msg + "\'");
			out.println(msg);
			out.flush();
		}
		
		@SuppressWarnings("unused")
		public void sendMsg(JSONObject msg) {
			sendMsg(msg.toString());
		}
		
		public void destroy() {
			try {
				sock.close();
				in.close();
				out.close();
			} catch (IOException e) {}
		}
		
		public UUID getID() {
			return id;
		}
		
		public Socket getSocket() {
			return sock;
		}
	};

	private boolean interrupted = false;
	
	private Server serv;
	private ClientObj client;
	
	public ClientWorker(Server serv, Socket clientSock) {
		this.client = new ClientObj(clientSock);
		this.serv = serv;
	}
	
	public void stop() {
		interrupted = true;
	}
	
	public void destroy() {
		stop();
		client.destroy();
	}
	
	public void setID(UUID id) {
		client.setID(id);
	}
	
	@Override
	public void run() {
		write("Started worker for IP " + client.getSocket());
		while (!interrupted && !Thread.currentThread().isInterrupted()) {
			Optional<JSONObject[]> ret = client.waitMsg();
			if (ret.isPresent()) {
				serv.parse(client.getID(), ret.get());
			}
		}
	}
	
	public InetAddress getIP() {
		return client.getSocket().getInetAddress();
	}
	
	private String shortID() {
		return client.getID().toString().substring(0, 5);
	}
	
	public void sendMsg(String msg) {
		client.sendMsg(msg);
	}
	
	public void sendMsg(JSONObject msg) {
		client.sendMsg(msg.toString());
	}
	
	public void sendMsg(Optional<JSONObject> msg) {
		if (msg.isPresent()) {
			client.sendMsg(msg.get().toString());
		}
	}

	private void write(String msg) {
		if (client.getID() != null) {
			Logger.write("[SERVER <-> \'" + shortID() + "\' | " + client.getSocket().getInetAddress() + "] " + msg);
		} else {
			Logger.write("[SERVER <-> " + client.getSocket().getInetAddress() + "] " + msg);
		}
	}
	
}
