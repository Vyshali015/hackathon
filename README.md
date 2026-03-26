A real-time collaborative C editor
A lightweight, real-time collaborative text editor built in C using socket programming and multithreading. The system enables multiple clients to connect to a centralized server, edit a shared document, and communicate simultaneously.

📌 Overview

Collab-C Editor demonstrates core concepts of:

Network programming using Winsock2
Concurrent client handling via multithreading
Synchronization using a lock-based editing mechanism

The project is designed as a scalable foundation for collaborative systems and real-time applications.

🚀 Key Features
Multi-client Collaboration
Supports multiple users editing a shared document in real time.
Authentication System
Basic username-password validation before accessing the editor.
Integrated Chat System
Users can communicate using /chat without interrupting editing.
Lock-based Editing Mechanism
Prevents simultaneous write conflicts by allowing one active editor at a time.
Merge Functionality
Combine edits using /merge command.
Offline Mode Simulation
Allows temporary disconnection simulation using /offline.
Document Analytics
Displays live word count and character count.
Rich Text Rendering (Client-side)
Supports formatted display:
Bold → *text*
Italic → _text_
Underline → ~text~
Highlight → !text!
Headings → #text
🛠️ Technology Stack
Language: C
Networking: Winsock2
Concurrency: Windows Threads (WinAPI)
Architecture: Client-Server Model
📐 System Design
Centralized server manages:
Active clients
Shared document state
Edit locks
Each client runs in a separate thread on the server
Broadcast mechanism ensures real-time updates across all clients
⚠️ Limitations
Localhost-based communication only (127.0.0.1)
Fixed client limit (5 users)
Basic authentication (hardcoded credentials)
No persistent storage (in-memory document only)
🔮 Future Enhancements
Persistent document storage (file/database integration)
Secure authentication (hashed credentials)
Cross-network support (LAN/Internet)
Conflict resolution using operational transforms or CRDTs
Graphical User Interface (GUI)
