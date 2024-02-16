#!/usr/bin/env python

import click
import os
import socket
import time
import uhal
import logging
from rich import print
from rich.table import Table
from rich.logging import RichHandler

from hermesmodules.tx_endpoints import tx_endpoints
from hermesmodules.rx_endpoints import rx_endpoints

from pprint import pprint
 

# from crappyhalclient import CrappyHardwareClient

# -----------------------------------------------------------------------------
# Utilities
def dict_to_table( vals: dict, **kwargs):

    t = Table(**kwargs)
    t.add_column('name')
    t.add_column('value', style='green')
    for k,v in vals.items():
        t.add_row(k,hex(v))
    return t

def read_regs(hw, reg_list):
    d = {}
    for r in reg_list:
        v = hw.read(r)
        d[r] = v
    return d

def read_and_print(hw, reg_list):
    d = {}
    for r in reg_list:
        v = hw.read(r)
        d[r] = v
    print(dict_to_table(d))


# -----------------------------------------------------------------------------
def dump_sub_regs(node):
    regs = {}
    for i in sorted(node.getNodes()):
        regs[i] = node.getNode(i).read()
    node.getClient().dispatch()

    return {k: v.value() for k, v in regs.items()}


# -----------------------------------------------------------------------------
def dump_reg(node):
    v = node.read()
    node.getClient().dispatch()
    return {node.getId(): v.value()}


class HermesController :
    def __init__(self, node):
        self.node = node

        self._load_info()
    

    def _load_info(self):
        magic = self.node.getNode('info.magic').read()
        self.node.getClient().dispatch()
        self.magic = magic.value()
        if self.magic != 0xdeadbeef:
            raise ValueError(f"Magic number check failed. Expected '0xdeadbeef', found '{hex(self.magic)}'")
        
        

        n_mgt = self.node.getNode('info.generics.n_mgts').read()
        n_src = self.node.getNode('info.generics.n_srcs').read()
        ref_freq = self.node.getNode('info.generics.ref_freq').read()
        self.node.getClient().dispatch()

        # Generics
        self.n_mgt = n_mgt.value()
        self.n_src = n_src.value()
        self.ref_freq = ref_freq.value()
        # Extra info
        self.n_srcs_p_mgt = self.n_src//self.n_mgt

    def get_node(self, id):
        return self.node.getNode(id)

    def get_nodes(self, regex):
        return self.node.getNodes(regex)
    
    def dispatch(self):
        self.node.getClient().dispatch()

    def sample_ctrs(self, seconds: int):
        self.node.getNode('samp.ctrl.samp').write(True)
        self.node.getNode('samp.ctrl.samp').write(False)
        self.node.getClient().dispatch()
        if seconds:
            time.sleep(seconds)
            self.node.getNode('samp.ctrl.samp').write(False)
            self.node.getNode('samp.ctrl.samp').write(False)
            self.node.getClient().dispatch()


    def sel_tx_mux(self, i: int):

        if i >= self.n_mgt:
            raise ValueError(f"Link {i} does not exist ({self.n_mgt})")

        self.get_node('tx_path.csr_tx_mux.ctrl.tx_mux_sel').write(i)
        self.dispatch()
        j = self.get_node('tx_path.csr_tx_mux.ctrl.tx_mux_sel').read()
        self.dispatch()
        print(f"Link {j} selected")


    def sel_tx_mux_buf(self, i: int):

        if i >= self.n_src:
            raise ValueError(f"Input buffer {i} does not exist ({self.n_src})")
        
        self.node.getNode('tx_path.tx_mux.csr.ctrl.sel_buf').write(i)
        self.node.getClient().dispatch()

    def sel_udp_core(self, i: int):

        if i >= self.n_mgt:
            raise ValueError(f"Link {i} does not exist ({self.n_mgt})")
        
        self.node.getNode('tx_path.csr_udp_core.ctrl.udp_core_sel').write(i)
        self.node.getClient().dispatch()


# N_MGT=4
# N_SRC=8
# N_SRCS_P_MGT = N_SRC//N_MGT
MAX_MGT=2
MAX_SRCS_P_MGT =16
mgts_all = tuple(str(i) for i in range(MAX_MGT))
# 
CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'])

# DEFAULT_MAP = {'connections': '${HERMESMODULES_SHARE}/config/etc/connections.xml'}
# CONTEXT_SETTINGS = {
#     'help_option_names': ['-h', '--help'],
#     'default_map': DEFAULT_MAP,
#     }

# -----------------
def validate_device(ctx, param, value):
    lDevices = ctx.obj.cm.getDevices()
    if value and (value not in lDevices):
        raise click.BadParameter(
            'Device must be one of '+
            ', '.join(["'"+lId+"'" for lId in lDevices])
            )
    return value
# -----------------

class HermesCliObj:
    
    def __init__(self):
        uhal.setLogLevelTo(uhal.LogLevel.WARNING)
        self.cm  = uhal.ConnectionManager('file://${HERMESMODULES_SHARE}/config/c.xml')
        self.hw = None
        self.ctrl_id = None
        self._controller = None

    @property
    def hermes(self):
        if self._controller is None:
            hw = self.cm.getDevice(self.ctrl_id)

            # Identify board
            is_zcu = hw.getNodes('tx.info')
            is_wib = hw.getNodes('info')

            if is_zcu:
                print("zcu mode")
                tx_mux = hw.getNode('tx')
            elif is_wib:
                print("wib mode")
                tx_mux = hw.getNode()
            else:
                raise ValueError(f"{self.ctrl_id} is neither a zcu nor a wib")

            self.hw = hw
            pprint(vars(self))
            self._controller = HermesController(tx_mux)

        return self._controller        


@click.group(chain=True, context_settings=CONTEXT_SETTINGS)
@click.option('-d', '--device', callback=validate_device, help="IPBus device")
# @click.pass_context
@click.pass_obj
def cli(obj, device):

    obj.ctrl_id = device

@cli.command()
@click.pass_obj
def addrbook(obj):

    t = Table(title="Control hosts")
    t.add_column('name')
    # t.add_column('addrtable', style='green')
    for h in obj.cm.getDevices():
        t.add_row(h)
    print(t)

    t = Table(title="Receivers")
    t.add_column('name')
    t.add_column('mac', style='green')
    t.add_column('ip', style='blue')
    t.add_column('port', style='blue')
    for h,d in rx_endpoints.items():
        t.add_row(h, f"0x{d['mac']:012x}", f"{d['ip']}", str(d['port']))
    print(t)

    t = Table(title="Transmitters")
    t.add_column('name')
    t.add_column('mac', style='green')
    t.add_column('ip', style='blue')
    t.add_column('port', style='blue')
    for h,d in tx_endpoints.items():
        t.add_row(h, f"0x{d['mac']:012x}", f"{d['ip']}", str(d['port']))
    print(t)


@cli.command()
@click.option('--nuke', is_flag=True, default=None)
@click.pass_obj
def reset(obj, nuke):

    hrms = obj.hermes

    if nuke is not None:        
        hrms.get_node('csr.ctrl.nuke').write(0x1)
        hrms.dispatch()

        time.sleep(0.1)

        hrms.get_node('csr.ctrl.nuke').write(0x0)
        hrms.dispatch()
    
    hrms.get_node('csr.ctrl.soft_rst').write(0x1)
    hrms.dispatch()

    time.sleep(0.1)

    hrms.get_node('csr.ctrl.soft_rst').write(0x0)
    hrms.dispatch()


@cli.command()
@click.option('--en/--dis', 'enable', default=None)
@click.option('--buf-en/--buf-dis', 'buf_en', default=None)
@click.option('--tx-en/--tx-dis', 'tx_en', default=None)
@click.option('-l', '--link', type=int, default=0)
@click.pass_obj
def enable(obj, enable, buf_en, tx_en, link):

    hrms = obj.hermes

    n_mgt = hrms.n_mgt

    if link >= n_mgt:
        raise ValueError(f"MGT {link} not instantiated")
    
    hrms.sel_tx_mux(link)

    print()

    tx_en = tx_en if tx_en is not None else enable
    buf_en = buf_en if buf_en is not None else enable

    if tx_en is not None:
        print(f"- {'Enabling' if tx_en else 'Disabling'} 'tx block'")
        hrms.get_node('tx_path.tx_mux.csr.ctrl.tx_en').write(tx_en)
    print()

    if buf_en is not None:
        print(f"- {'Enabling' if buf_en else 'Disabling'} 'input buffers'")
        hrms.get_node('tx_path.tx_mux.csr.ctrl.en_buf').write(buf_en)

    time.sleep(0.1)

    if enable is not None:
        print(f"- {'Enabling' if enable else 'Disabling'} 'mux'")
        hrms.get_node('tx_path.tx_mux.csr.ctrl.en').write(enable)


    
    hrms.dispatch()


    top_ctrl = dump_sub_regs(hrms.get_node('tx_path.tx_mux.csr.ctrl'))

    print(
        dict_to_table(top_ctrl, title=f'Link {link} ctrls', show_header=False), 
    )


@cli.command("mux-config")
@click.argument('detid', type=int)
@click.argument('crate', type=int)
@click.argument('slot', type=int)
@click.option('-l', '--link', type=int, default=0)
@click.pass_obj
def mux_config(obj, detid, crate, slot, link):
    """Configure the UDP blocks """

    hrms = obj.hermes

    hrms.sel_tx_mux(link)

    hrms.get_node('tx_path.tx_mux.mux.ctrl.detid').write(detid)
    hrms.get_node('tx_path.tx_mux.mux.ctrl.crate').write(crate)
    hrms.get_node('tx_path.tx_mux.mux.ctrl.slot').write(slot)
    
    mux_ctrl = dump_sub_regs(hrms.get_node('tx_path.tx_mux.mux.ctrl'))

    print(
        dict_to_table(mux_ctrl, title=f'Link {link} mux cfg', show_header=False), 
    )


@cli.command("udp-config")
@click.argument('src_id', type=click.Choice(tx_endpoints.keys()))
@click.argument('dst_id', type=click.Choice(rx_endpoints.keys()))
@click.option('-l', '--link', type=int, default=0)
@click.pass_obj
def udp_config(obj, src_id, dst_id, link):
    """Configure the UDP blocks """

    filter_control = 0x07400307
    hrms = obj.hermes

    if link >= hrms.n_mgt:
        raise ValueError(f"Link {link} not instantiated")


    dst = rx_endpoints[dst_id]
    src = tx_endpoints[src_id]

    udp_core_ctrl = f'tx_path.udp_core.udp_core_control'

    hrms.get_node(f'{udp_core_ctrl}.ctrl.filter_control').write(filter_control)

    # Our IP address = 10.73.139.23
    # print(f"Our ip address: {socket.inet_ntoa(src['ip'].to_bytes(4, 'big'))}")
    src_u32 = int.from_bytes(socket.inet_aton(src['ip']),"big")
    print(f"Our ip address: {src['ip']} (0x{src_u32:08x})")
    
    hrms.get_node(f'{udp_core_ctrl}.src_addr_ctrl.src_ip_addr').write(src_u32) 

    # Their IP address = 10.73.139.23
    # print(f"Their ip address: {socket.inet_ntoa(dst['ip'].to_bytes(4, 'big'))}")
    dst_u32 = int.from_bytes(socket.inet_aton(dst['ip']),"big")
    print(f"Their ip address: {dst['ip']} (0x{dst_u32:08x})")
    hrms.get_node(f'{udp_core_ctrl}.ctrl.dst_ip_addr').write(dst_u32) 
    
    # Our MAC address
    # Dest MAC address
    print(f"Our mac address: 0x{src['mac']:012x}")
    hrms.get_node(f'{udp_core_ctrl}.src_addr_ctrl.src_mac_addr_lower').write(src['mac'] & 0xffffffff) 
    hrms.get_node(f'{udp_core_ctrl}.src_addr_ctrl.src_mac_addr_upper').write((src['mac'] >> 32) & 0xffff) 

    # Dest MAC address
    print(f"Their mac address: 0x{dst['mac']:012x}")
    hrms.get_node(f'{udp_core_ctrl}.ctrl.dst_mac_addr_lower').write(dst['mac'] & 0xffffffff) 
    hrms.get_node(f'{udp_core_ctrl}.ctrl.dst_mac_addr_upper').write((dst['mac'] >> 32) & 0xffff) 

    # Ports
    hrms.get_node(f'{udp_core_ctrl}.src_addr_ctrl.src_port').write(src['port']) 
    hrms.get_node(f'{udp_core_ctrl}.ctrl.dst_port').write(dst['port']) 

    hrms.dispatch()


@cli.command("zcu-src-config")
@click.option('-l', '--link', type=int, default=0)
@click.option('-n', '--en-n-src', type=click.IntRange(0, MAX_SRCS_P_MGT), default=1)
@click.option('-d', '--dlen', type=click.IntRange(0, 0xfff), default=0x383)
@click.option('-r', '--rate-rdx', type=click.IntRange(0, 0x3f), default=0xa)
@click.pass_obj
def zcu_src_config(obj, link, en_n_src, dlen, rate_rdx):
    """Configure trivial data sources"""

    hw = obj.cm.getDevice(obj.ctrl_id)
    #hw = obj.hw
    hrms = obj.hermes
    
    # n_mgt = obj.n_mgt
    # n_src = obj.n_src
    # n_srcs_p_mgt = n_src//n_mgt

    #print(hw)
    #pprint(vars(obj))
    #pprint(vars(hrms))
    #print(hw)    

    if link >= hrms.n_mgt:
        raise ValueError(f"MGT {link} not instantiated")
    
    if en_n_src > hrms.n_srcs_p_mgt:
        raise ValueError(f"{en_n_src} must be lower than the number of generators per link ({hrms.n_srcs_p_mgt})")

    for i in range(hrms.n_srcs_p_mgt):
        src_id = hrms.n_srcs_p_mgt*link+i
        hw.getNode('src.csr.ctrl.sel').write(src_id)
        src_en = (i<en_n_src)
        print(f'Configuring generator {src_id} : {src_en}')
        #data_gen = hw.getNode("src.data_src.src.params.write")
        #data_gen.getNode("gap").write(1)
        #hw.getNode('src.data_src.src.csr.ctrl.start_stop').write(src_en)
        #if not src_en:
        #    continue
        ## Number of words per block
        #hw.getNode('src.ctrl.dlen').write(dlen)
        ## ????
        #hw.getNode('src.ctrl.rate_rdx').write(rate_rdx) 
        hw.dispatch()


    regs = {}
    for i in range(hrms.n_srcs_p_mgt):
        hw.getNode('src.csr.ctrl.sel').write(i)
        src_regs = dump_sub_regs(hw.getNode('src.csr'))
        data_src_regs = dump_sub_regs(hw.getNode('src.data_src'))

        grid = Table.grid()
        grid.add_column("ctrl")
        grid.add_column("stat")
        grid.add_row(
            dict_to_table(src_regs, title='src_ctrl', show_header=False),
            #dict_to_table(data_src_regs, title='data_src_ctrl', show_header=False)
        )
        for j in range(hrms.n_srcs_p_mgt):
            grid.add_row(
                dict_to_table(data_src_regs, title='data_src_ctrl', show_header=False)

            )
        print(f'Link {i}')
        print(grid)

        #regs[i] =  dump_sub_regs(hw.getNode('src.csr.stat.no_of_src_blocks'))

    # Create the summary table
    t = Table()

    

    # Add 1 column for the reg name, and as many as the number of sources
    #t.add_column('name')
    # for j in range(hrms.n_srcs_p_mgt):
    #     t.add_column(f'Src {j}', style='green')

    # for n in hw.getNode('src').getNodes():
    #     t.add_row(n, *[hex(regs[i][n]) for i in range(hrms.n_srcs_p_mgt)])

    
    #print(t)

@cli.command("fakesrc-config")
@click.option('-l', '--link', type=int, default=0)
@click.option('-n', '--n-src', type=click.IntRange(0, MAX_SRCS_P_MGT), default=1)
@click.option('-s', '--src', type=click.IntRange(0, MAX_SRCS_P_MGT), default=None, multiple=True)
@click.option('-k', '--data-len', type=click.IntRange(0, 0xfff), default=0x383)
@click.option('-r', '--rate-rdx', type=click.IntRange(0, 0x3f), default=0xa)
@click.pass_obj
def fakesrc_config(obj, link, n_src, src, data_len, rate_rdx):
    """Configure trivial data sources"""

    hrms = obj.hermes

    all_srcs = set(range(hrms.n_srcs_p_mgt))
    en_srcs = set(src) 

    if en_srcs != set.intersection(all_srcs, en_srcs):
        raise ValueError("AAARGH")

    # print(src)
    # return

    hrms.sel_tx_mux(link)

    if n_src > hrms.n_srcs_p_mgt:
        raise ValueError(f"{n_src} must be lower than the number of generators per link ({n_srcs_p_mgt})")

    was_en = hrms.get_node('tx_path.tx_mux.csr.ctrl.en_buf').read()
    # disable buffer before reconfiguring "or bad things will happen"
    hrms.get_node('tx_path.tx_mux.csr.ctrl.en_buf').write(0x0)
    hrms.dispatch()
    
    for src_id in range(hrms.n_srcs_p_mgt):
        hrms.sel_tx_mux_buf(src_id)

        # src_en = (src_id<n_src)
        src_en = (src_id in en_srcs)
        print(f'Configuring generator {src_id} : {src_en}')
        # hw.write(f'tx.buf.ctrl.fake_en', src_en)
        hrms.get_node('tx_path.tx_mux.buf.ctrl.fake_en').write(src_en)
        if not src_en:
            continue
        ## ????
        hrms.get_node('tx_path.tx_mux.buf.ctrl.dlen').write(data_len)
        ## ????
        hrms.get_node('tx_path.tx_mux.buf.ctrl.rate_rdx').write(rate_rdx) 
        hrms.dispatch()

    hrms.get_node('tx_path.tx_mux.csr.ctrl.en_buf').write(was_en.value())
    hrms.dispatch()


@cli.command()
@click.pass_obj
@click.option('-l', '--links', 'sel_links', type=click.Choice(mgts_all), multiple=True, default=None)
@click.option('-s', '--seconds', type=int, default=0)
@click.option('-u/-U', '--show-udp/--hide-udp', 'show_udp', default=True)
@click.option('-b/-B', '--show-buf/--hide-buf', 'show_buf', default=True)

def stats(obj, sel_links, seconds, show_udp, show_buf):
    """Simple program that greets NAME for a total of COUNT times."""

    hrms = obj.hermes

    mgts = list(range(hrms.n_mgt))
    
    # deal with defaults
    if not sel_links:
        sel_links = mgts
    else:
        sel_links = [int(s) for s in sel_links]


    # Check for existance
    if not set(sel_links).issubset(mgts):
        print(sel_links, mgts)
        raise ValueError(f"MGTs {set(sel_links)-set(mgts)} are not instantiated")
    
    # mgts = [int(s) for s in (mgts if mgts else [0])]

    print(f"Sampling hermes counters for {seconds}s")
    hrms.sample_ctrs(seconds)

    info_data = dump_sub_regs(hrms.get_node('info'))
    print(dict_to_table(info_data, title='hermes info', show_header=False))

    # n_srcs_p_mgt = n_src//n_mgt

    for i in sel_links:
        print(f'---Tx Mux {i} Status---')

        hrms.sel_tx_mux(i)


        # cli control registers
        top_ctrl = dump_sub_regs(hrms.get_node('tx_path.tx_mux.csr.ctrl'))
        top_stat = dump_sub_regs(hrms.get_node('tx_path.tx_mux.csr.stat'))
        ctrl_mux = dump_sub_regs(hrms.get_node('tx_path.tx_mux.mux.ctrl'))
        stat_mux = dump_sub_regs(hrms.get_node('tx_path.tx_mux.mux.stat'))

        grid = Table.grid()
        grid.add_column("ctrl")
        grid.add_column("stat")
        grid.add_row(
            dict_to_table(top_ctrl, title='tx_mux ctrl', show_header=False), 
            dict_to_table(top_stat, title='tx mux stat', show_header=False),
            dict_to_table(ctrl_mux, title="mux ctrl", show_header=False),
            dict_to_table(stat_mux, title="mux stat", show_header=False),
        )
        print(grid)
        

        if show_udp:
            hrms.sel_udp_core(i)
            ctrl_udp_src = dump_sub_regs(hrms.get_node(f'tx_path.udp_core.udp_core_control.src_addr_ctrl'))
            ctrl_udp = dump_sub_regs(hrms.get_node(f'tx_path.udp_core.udp_core_control.ctrl'))
            ctrl_flt_udp = dump_sub_regs(hrms.get_node(f'tx_path.udp_core.udp_core_control.ctrl.filter_control'))
            if hrms.get_nodes(f'tx_path.udp_core.udp_core_control.rx_packet_counters'):
                print("New tx counters found")
                stat_rx_udp = dump_sub_regs(hrms.get_node(f'tx_path.udp_core.udp_core_control.rx_packet_counters'))
                stat_tx_udp = dump_sub_regs(hrms.get_node(f'tx_path.udp_core.udp_core_control.tx_packet_counters'))
            else:
                print("No new tx counters found")
                stat_rx_udp = dump_sub_regs(hrms.get_node(f'tx_path.udp_core.udp_core_control.tx_packet_counters'))
                stat_tx_udp = {'-':0}


            ctrl_srcdst = {}
            ctrl_srcdst['src_ip'] = ctrl_udp_src['src_ip_addr']
            ctrl_srcdst['dst_ip'] = ctrl_udp['dst_ip_addr']
            ctrl_srcdst['src_mac'] = (ctrl_udp_src['src_mac_addr_upper'] << 32) + ctrl_udp_src['src_mac_addr_lower']
            ctrl_srcdst['dst_mac'] = (ctrl_udp['dst_mac_addr_upper'] << 32) + ctrl_udp['dst_mac_addr_lower']
            ctrl_srcdst['src_port'] = ctrl_udp_src['src_port']
            ctrl_srcdst['dst_port'] = ctrl_udp['dst_port']

            grid = Table.grid()
            grid.add_column("ctrl")
            grid.add_column("stat")
            grid.add_row(
                # dict_to_table(ctrl_udp, title="udp ctrl", show_header=False),
                dict_to_table(ctrl_srcdst, title="udp src/dst", show_header=False),
                dict_to_table(ctrl_flt_udp, title="udp filter", show_header=False),
                dict_to_table(stat_rx_udp, title="udp rx stat", show_header=False),
                dict_to_table(stat_tx_udp, title="udp tx stat", show_header=False),
            )
            print(grid)

        if show_buf:
            ibuf_stats = {}

            src_ids = tuple(range(hrms.n_srcs_p_mgt))
            for j in src_ids:
                # hw.write('tx.mux.csr.ctrl.sel_buf',j)
                hrms.sel_tx_mux_buf(j)
                s =  dump_sub_regs(hrms.get_node('tx_path.tx_mux.buf'))
                s['blk_acc'] = (s['blk_acc_h']<<32)+s['blk_acc_l']
                s['blk_oflow'] = (s['blk_oflow_h']<<32)+s['blk_oflow_l']
                s['blk_rej'] = (s['blk_rej_h']<<32)+s['blk_rej_l']
                s['ts'] = (s['ts_h']<<32)+s['ts_l']
                s['vol'] = (s['vol_h']<<32)+s['vol_l']
                s['blk_longlast'] = (s['blk_longlast_h']<<32)+s['blk_longlast_l']
                s['blk_lastnotval'] = (s['blk_lastnotval_h']<<32)+s['blk_lastnotval_l']

                for k in tuple(s.keys()):
                    for n in ('blk_acc_', 'blk_oflow_', 'blk_rej_', 'ts_', 'vol_', 'blk_longlast_', 'blk_lastnotval_'):
                        if k.startswith(n):
                            del s[k]

                for k in ('ctrl', 'stat', 'buf_mon'):
                    del s[k]

                    
                ibuf_stats[j] = s

            # Create the summary table
            t = Table()

            # Add 1 column for the reg name, and as many as the number of sources
            t.add_column('name')
            for j in src_ids:
                t.add_column(f'Buf {j}', style='green')

            # Unify the reg list (useless?)
            reg_names = set()
            for k,v in ibuf_stats.items():
                reg_names = reg_names.union(v.keys())
            
            for n in sorted(reg_names):
                t.add_row(n,*(hex(ibuf_stats[j][n]) for j in src_ids))
            print(t)
            

if __name__ == '__main__':
    FORMAT = "%(message)s"
    logging.basicConfig(
        level="INFO", format=FORMAT, datefmt="[%X]", handlers=[RichHandler()]
    )
    cli(obj=HermesCliObj())
