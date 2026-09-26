# FeralHeart Non-Commercial + Supporter Access License [FH(NC+SA)L]

Except where otherwise noted, the FeralHeart source code in this repository is released under the terms below:

```
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to use, copy, modify, merge, publish, and distribute copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

1. Non-Commercial Use. The Software and derivative works (“Projects”) may not be used for commercial purposes. “Commercial purposes” include (without limitation) selling or licensing the game, advertising, paywalls, subscriptions, donations tied to access or in-game benefits, or any activity intended for commercial advantage or monetary compensation. Commercial purposes do not include voluntary support or donations not tied to exclusive product access, free promotions, or non-monetary acknowledgements, provided they do not grant gameplay benefits unavailable to non-supporters.

2. Supporter Access Exception (Patreon/Ko-fi/etc). Notwithstanding Section 1, Projects may run an optional “Supporter Access” program (e.g., Patreon) under all of the following conditions:

- Scope of Access: Supporters may receive pre-release materials only (e.g., alpha/beta/dev builds, test servers, WIP assets, dev logs).

- No Final Paywall: The released/stable game must be free to everyone with no required payment.

- No Microtransactions: No in-game purchases, premium currency, paid cosmetics, DLC, loot boxes, battle passes, or paid power/progression of any kind.

- Timed Public Release: Any build or content provided to supporters must be intended for free public release within a reasonable timeframe, defined as within 120 days of first supporter availability.

- Parity of Content: No permanent supporter-exclusive game content or features. Early access is okay; permanent exclusivity is not.

- Advertising: No ads or sponsor placements inside the game client. (Sponsor thanks on a project page or Patreon post are okay; keep the game client ad-free.)

If any condition above is not met, the Supporter Access program is not permitted under this license.

3. Acceptable Content. Projects may not be used to create, host, or distribute illegal or inappropriate content, including but not limited to hate speech, harassment, or sexually explicit material.

4. Attribution. Appropriate credit must be given by including a notice that the project is based on the FeralHeart codebase and a link to the repository.

5. Third-Party Components. This license applies only to the FeralHeart code. It does not grant rights to third-party components. You are responsible for complying with all third-party licenses and restrictions.

6. Warranty & Liability Disclaimer. The Software is provided "as is", without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, or non-infringement. The authors shall not be liable for any claim, damages, or other liability arising from the Software or its use, even if advised of the possibility of such damage.

By using the Software you agree to these terms.
```

If you prefer a standard OSI license, you may fork and relicense your contributions under a standard non‑commercial variant if and only if it is compatible with all third‑party components you include. When in doubt, keep the FHNCL text above in your fork.

## Examples related to the FHNCL + FHSAL:

✅ Allowed
- A fan spin-off offers a $3/month Patreon that grants early access to beta builds and WIP posts. Every early build (or its successor) is released for free to the public within 120 days. The public game is free to download and play and has no microtransactions.

❌ Unallowed (Microtransactions / Permanent Exclusives)
- A derivative project charges $10 to unlock a mount and skins in the live game and keeps an extra dungeon permanently Patreon-only. This violates No Microtransactions and Parity of Content.

❌ Unallowed (Selling the Base Game)
- A developer sells the base game for $5 (Steam, itch.io, direct, etc.). This violates Non-Commercial Use and No Final Paywall; the released game must be free.


## Third‑party components and licenses
- Ogre SDK 1.7.4 – OGRE is under an MIT‑style license in modern versions; verify the exact 1.7.4 terms.
- OIS – zlib/libpng license.
- Boost 1.57 – Boost Software License.
- RakNet 4.023 – modified BSD license.
- zlib / zziplib – zlib/libpng license.
- irrKlang – proprietary; redistribution typically not permitted; obtain from the vendor.
- FMOD (debug config only) – proprietary; obtain from the vendor.
- SkyX and PagedGeometry – open‑source; verify license terms in their repositories.
- MySQL++ – LGPL; MySQL Connector/C – GPL with FOSS exception; verify versioned terms.

You must obtain these libraries yourself, build the x86 variants, and comply with their respective licenses. Do not commit proprietary SDKs or binaries into this repository.