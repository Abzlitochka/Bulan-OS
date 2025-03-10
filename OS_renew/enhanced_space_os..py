import os
import time
import random
import platform
from datetime import datetime
from colorama import init, Fore, Back, Style

class SpaceVehicleOS:
    def __init__(self):
        init(autoreset=True)  # Initialize colorama
        self.running = True
        self.memory_usage = [30, 15, 25, 40, 35, 45, 60, 70]
        self.network_traffic = [40, 20, 60, 15, 30, 45, 55, 35, 25]
        self.packets = {
            "packet_3": {"loss": 0, "total": 133},
            "packet_8": {"loss": 0, "total": 98},
            "packet_5": {"loss": 0, "total": 112},
            "packet_22": {"loss": 0, "total": 76},
            "packet_14": {"loss": 2, "total": 154}
        }
        self.status = {
            "Engine": True,
            "OBC": True,
            "Transceiver": True,
            "Memory Module": True,
            "DEI": True,
            "Power controller": True,
            "Solar panels": True,
            "Thrusters": True,
            "ACT": True,
            "Sun Sensors": True,
            "Reaction wheels": True,
            "Magnetic torquers": True,
            "Gyroscopes": True
        }
        self.current_tasks = "None"
        self.ping = "5ms"
        self.state = "Online"
        self.system_id = "12000928341176"
        self.engine_usage = 0
        self.speed = 0
        self.power_source = "WIRED"
        self.iteration = 0
        
        # Terminal size check
        self.get_terminal_size()
        
    def get_terminal_size(self):
        try:
            columns, rows = os.get_terminal_size()
            self.term_width = columns
            self.term_height = rows
        except:
            self.term_width = 80
            self.term_height = 30
    
    def clear_screen(self):
        if platform.system() == "Windows":
            os.system("cls")
        else:
            os.system("clear")
    
    def update_data(self):
        # Update memory usage with realistic pattern
        self.memory_usage.pop(0)
        new_value = self.memory_usage[-1] + random.randint(-10, 10)
        new_value = max(10, min(90, new_value))  # Keep within 10-90% range
        self.memory_usage.append(new_value)
        
        # Update network traffic with realistic pattern
        self.network_traffic.pop(0)
        new_value = self.network_traffic[-1] + random.randint(-15, 15)
        new_value = max(5, min(95, new_value))  # Keep within 5-95% range
        self.network_traffic.append(new_value)
        
        # Occasionally update system status
        if random.random() < 0.05:  # 5% chance each iteration
            system = random.choice(list(self.status.keys()))
            self.status[system] = not self.status[system]
        
        # Update packet losses
        for packet in self.packets:
            if random.random() < 0.2:  # 20% chance to change loss value
                self.packets[packet]["loss"] = random.randint(0, 5)
                
        # Update speed and engine usage occasionally
        if self.iteration % 5 == 0:
            self.speed = random.randint(0, 30)
            self.engine_usage = random.randint(0, 40) if self.speed > 0 else 0
        
        self.iteration += 1

    def draw_ascii_bar(self, value, width=20, char="#"):
        filled = int((value / 100) * width)
        return char * filled + " " * (width - filled)
    
    def draw_ascii_graph(self, data_points, width=40, height=5):
        graph = []
        max_val = max(data_points)
        min_val = min(data_points)
        range_val = max(1, max_val - min_val)
        
        for y in range(height):
            line = []
            threshold = min_val + (height - y - 1) * range_val / height
            
            for point in data_points:
                if point >= threshold:
                    line.append("*")
                else:
                    line.append(" ")
            graph.append("".join(line))
            
        return graph
    
    def create_ascii_globe(self, size=10):
        globe = []
        for y in range(size):
            line = []
            for x in range(size * 2):
                # Basic circle equation
                distance = ((x - size) ** 2 + (y - size/2) ** 2) ** 0.5
                if distance <= size/2:
                    # Create a pattern inside the globe
                    if (x + y) % 3 == 0:
                        line.append(".")
                    else:
                        line.append(" ")
                else:
                    line.append(" ")
            globe.append("".join(line))
        return globe
    
    def display_ui(self):
        self.clear_screen()
        self.get_terminal_size()
        
        # Current time
        now = datetime.now()
        time_str = now.strftime("%H:%M:%S")
        date_str = now.strftime("%d %B %Y")
        
        # Header
        print(Fore.MAGENTA + "╔" + "═" * (self.term_width - 2) + "╗")
        print(Fore.MAGENTA + "║" + Fore.BLUE + " SPACE VEHICLE OPERATING SYSTEM " + " " * (self.term_width - 33) + Fore.MAGENTA + "║")
        print(Fore.MAGENTA + "╠" + "═" * (self.term_width - 2) + "╣")
        
        # Time and basic info
        print(Fore.MAGENTA + "║" + Fore.CYAN + f" {time_str} " + Fore.WHITE + f"| Date: {date_str} | GMT +5 | BULAN" + " " * (self.term_width - 50) + Fore.MAGENTA + "║")
        print(Fore.MAGENTA + "║" + Fore.CYAN + f" Power: {self.power_source} " + Fore.WHITE + f"| Speed: {self.speed} km/h | Engine: {self.engine_usage}%" + " " * (self.term_width - 60) + Fore.MAGENTA + "║")
        print(Fore.MAGENTA + "╠" + "═" * (self.term_width - 2) + "╣")
        
        # Split into columns
        col_width = (self.term_width - 6) // 3
        
        # Create columns content
        # Column 1: System Status
        col1 = [Fore.CYAN + " SYSTEM STATUS " + Fore.WHITE]
        for system, status in self.status.items():
            status_str = Fore.GREEN + "OK" if status else Fore.RED + "FAIL"
            col1.append(f" {system}: {status_str}")
        
        # Column 2: Memory & Network Graphs
        memory_graph = self.draw_ascii_graph(self.memory_usage, width=col_width-4, height=5)
        network_graph = self.draw_ascii_graph(self.network_traffic, width=col_width-4, height=5)
        
        col2 = [Fore.CYAN + " MEMORY USAGE " + Fore.WHITE]
        col2.extend(memory_graph)
        col2.append("")
        col2.append(Fore.CYAN + " NETWORK TRAFFIC " + Fore.WHITE)
        col2.extend(network_graph)
        
        # Column 3: Network Status & Globe
        globe = self.create_ascii_globe(size=8)
        
        col3 = [Fore.CYAN + " NETWORK STATUS " + Fore.WHITE]
        col3.append(f" State: {Fore.GREEN}{self.state}")
        col3.append(f" Ping: {Fore.GREEN}{self.ping}")
        col3.append(f" ID: {Fore.GREEN}{self.system_id}")
        col3.append("")
        col3.extend(globe)
        
        # Determine the max column height
        max_height = max(len(col1), len(col2), len(col3))
        
        # Pad columns to the same height
        col1 += [""] * (max_height - len(col1))
        col2 += [""] * (max_height - len(col2))
        col3 += [""] * (max_height - len(col3))
        
        # Display the columns
        for i in range(max_height):
            line = Fore.MAGENTA + "║ " + col1[i].ljust(col_width) + " │ " + col2[i].ljust(col_width) + " │ " + col3[i].ljust(col_width) + " " + Fore.MAGENTA + "║"
            print(line)
        
        print(Fore.MAGENTA + "╠" + "═" * (self.term_width - 2) + "╣")
        
        # Packet Status
        print(Fore.MAGENTA + "║" + Fore.CYAN + " PACKET STATUS " + " " * (self.term_width - 16) + Fore.MAGENTA + "║")
        
        packet_line = Fore.MAGENTA + "║ "
        for packet, data in self.packets.items():
            loss_color = Fore.GREEN if data["loss"] == 0 else Fore.RED
            packet_info = f"{packet}: {loss_color}{data['loss']}%{Fore.WHITE} loss "
            packet_line += packet_info
        
        packet_line += " " * (self.term_width - len(packet_line) - 1) + Fore.MAGENTA + "║"
        print(packet_line)
        
        # Footer with tasks and keyboard
        print(Fore.MAGENTA + "╠" + "═" * (self.term_width - 2) + "╣")
        print(Fore.MAGENTA + "║" + Fore.CYAN + " CURRENT TASKS: " + Fore.WHITE + self.current_tasks + " " * (self.term_width - 16 - len(self.current_tasks)) + Fore.MAGENTA + "║")
        print(Fore.MAGENTA + "║" + Fore.CYAN + " ENTER TASK: " + Fore.WHITE + "_" * 20 + " " * (self.term_width - 33) + Fore.MAGENTA + "║")
        
        # ASCII Keyboard
        keyboard = " 1234567890-+QWERTYUIOP[]ASDFGHJKL;'ZXCVBNM<>/ "
        print(Fore.MAGENTA + "║" + Fore.CYAN + " " + keyboard + " " * (self.term_width - 2 - len(keyboard)) + Fore.MAGENTA + "║")
        
        print(Fore.MAGENTA + "╚" + "═" * (self.term_width - 2) + "╝")
        print(Fore.WHITE + "Press Ctrl+C to exit")

    def run(self):
        try:
            while self.running:
                self.update_data()
                self.display_ui()
                time.sleep(0.5)  # Update twice per second for smoother animation
        except KeyboardInterrupt:
            self.clear_screen()
            print(Fore.CYAN + "Exiting Space Vehicle OS...")
            print(Fore.WHITE + "Thank you for using Space Vehicle OS.")

if __name__ == "__main__":
    try:
        print("Starting Space Vehicle Operating System...")
        print("Initializing systems...")
        time.sleep(1)
        print("Loading interface...")
        time.sleep(1)
        
        os_system = SpaceVehicleOS()
        os_system.run()
    except Exception as e:
        print(f"Error: {str(e)}")
        print("\nTo fix this error:")
        print("1. Make sure you have colorama installed: pip install colorama")
        print("2. Run the program in a terminal with enough width (at least 80 columns)")
        print("3. If on Windows, try running in PowerShell or Command Prompt")