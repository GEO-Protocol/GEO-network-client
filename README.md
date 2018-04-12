# GEO Project
Stack of `open-source`, `lightweight`, `low-level` protocols for `decentralised` `p2p` `multihop` financial cooperation.

The main goal of a project is to provide solution for sending payments directly, and indirectly (through middleware participants), even across different payment networks with different infrastructures, safely and in a predictable manner.
Our mission is to provide base protocols for decentralised financial cooperation, that would power the Financial Internet, as TCP does for the Internet itself.

[<img src="https://camo.githubusercontent.com/aebd3cdcab53bdc6fbd1362bcc78a0812677c627/68747470733a2f2f706174726f6c617669612e6769746875622e696f2f74656c656772616d2d62616467652f666f6c6c6f772e706e67">](https://t.me/geopeopleeng) — International Community. 

[<img src="https://camo.githubusercontent.com/aebd3cdcab53bdc6fbd1362bcc78a0812677c627/68747470733a2f2f706174726f6c617669612e6769746875622e696f2f74656c656772616d2d62616467652f666f6c6c6f772e706e67">](http://t.me/geopeoplegroup) — Ukrainian Community.


## Summary
Protocol | Purpose | Brief| State
--- | --- | --- | ---
**Trust&nbsp;Lines** | Safe, predictable and minimalistic,  fault-tolerance, fraud-resistant, replication-friendly, activities accounting. |  `Decentralised` `Distributed` **`QRCrypto^`** | `Draft`
**Routing** / **NAT&nbsp;Traversing** | Fast, network fault tolerance, participants discovering and traffic routing. | `Decentralised` `Distributed` | `WiP^`
**Payment** | Fast, safe, double-spending resistant, distributed multi-attendee payment crypto-protocol with **time predictable 100% participants consensus.** | `Decentralised` `Distributed` **`QRCrypto^`** | `Draft`
**Payment&nbsp;flow prediction** | Near real-time (depends on network segment) participants abilities forecasting towards other participants. | `Decentralised` `Distributed` | `Draft`
**Cycles&nbsp;discovering and processing** | Automatic, atomic, discovering and overlapping circular debts for up to 6 participants included. | `Decentralised` `Distributed` `Crypto` | `Draft`
 
*^ QR Crypto — quantum resistant cryptography. In most cases, solutions are based on top of [Lamport signature.](https://en.wikipedia.org/wiki/Lamport_signature)*

*^WiP — work in progress*.


## Design decisions summary 
* **Efficient decentralisation** is distribution. Provided solutions are **not block-chain based**, doesn't imply all the participants to store and process all operations, doesn't rely on network majority, and doesn't need global network consensus to proceed. Another one focus point for the project is to keep protocols as efficient as possible, so they would be able to run on low-power devices (for example, modern cellphones) and IoT devices.

* **Modularity** — almost all provided protocols are capable to work independently. The common dependencies are (I) trust lines mechanics and (II) low-level information transfer approaches. Also, there are several common crypto-techniques shared between protocols (III).

* **Minimalism** — provided solutions strives to be as simple as possible, and to solve one and only one problem. In case if the project needs solution for another one problem — new research branch should be open. We believe, that good modularity will play crucial role in efficient crypt-analising and protocols popularisation. 

* **Lightweightness** — all present protocols are capable to perform well on modern cellphones, and/or IoT devices, thus extends the frontier of modern p2p systems.


## Commisions
The GEO Project provides open protocol suite for sending payments across different ledgers. It doesn't imply attendees to charge any protocol-fixed commissions, and doesn't provide any low-level primitives for this process (for example, any network-relevant token). But, the attendees are free to set custom requirements for payment processing. One important notice here, that routing protocol is now not able (yet) to choose the cheapest path in the network. 


## Related projects
* **[geo-pay.net](https://geo-pay.net)** — first payment service, built on top of GEO Project technologies.


## Contribution
**- Why should I contribute to GEO Project?**

GEO Projects make it easier for people to transfer assets through the Internet in the p2p manner, without (or with significantly less) commissions, fast, and in a predictable manner. Unlike many other decentralised payment systems, it is able to process significant transactions flow and to distribute it through the whole network. Also, the project itself aims to be ready for post-quantum epoch, and looks for stronger cryptography than most of other decentralised systems.

**- How I can help?**

Today the project is looking for:
* C/C++ developers for joining core protocols designing and developing;
* Crypt-analitycs for analising core protocols for vulnerabilities and providing solutions for the project's challenges;


**- How I can join?**

Please, refer to the one of public Telegram groups, and find somebody there. 
