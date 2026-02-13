/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { cryptoFramework } from '@kit.CryptoArchitectureKit';
import { buffer } from '@kit.ArkTS';

export class HashUtils {
  public static instance: HashUtils;


  /* SHA256 logical privates */
  private rotateRight(n : number, x : number) {
    return ((x >>> n) | (x << (32 - n)));
  }

  private choice(x : number, y : number, z : number) {
    return ((x & y) ^ (~x & z));
  }

  private majority(x : number, y : number, z : number) {
    return ((x & y) ^ (x & z) ^ (y & z));
  }

  private sha256Sigma0(x : number) {
    return (this.rotateRight(2, x) ^ this.rotateRight(13, x) ^ this.rotateRight(22, x));
  }

  private sha256Sigma1(x : number) {
    return (this.rotateRight(6, x) ^ this.rotateRight(11, x) ^ this.rotateRight(25, x));
  }

  private shaSigma(x : number) {
    return (this.rotateRight(7, x) ^ this.rotateRight(18, x) ^ (x >>> 3));
  }

  private sha256Sigma(x : number) {
    return (this.rotateRight(17, x) ^ this.rotateRight(19, x) ^ (x >>> 10));
  }

  private sha256Expand(w : Array<number>, j : number) {
    return (w[j & 0x0f] += this.sha256Sigma(w[(j + 14) & 0x0f]) + w[(j + 9) & 0x0f] +
    this.shaSigma(w[(j + 1) & 0x0f]));
  }

  /* Hash constant words K: */
  public K256 = [
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
  ];

  /* Add 32-bit integers with 16-bit operations (bug in some JS-interpreters:
  overflow) */
  private safeAdd(x : number, y : number) {
    let lsw = (x & 0xffff) + (y & 0xffff);
    let msw = (x >> 16) + (y >> 16) + (lsw >> 16);
    return (msw << 16) | (lsw & 0xffff);
  }
  public ihash:Array<number> = new Array(8);
  public count:Array<number> = new Array(2);
  public buffer:Array<number> = new Array(64);
  /* Initialise the SHA256 computation */
  private sha256Init() {
    this.ihash = new Array(8);
    this.count = new Array(2);
    this.buffer = new Array(64);
    this.count[0] = this.count[1] = 0;
    this.ihash[0] = 0x6a09e667;
    this.ihash[1] = 0xbb67ae85;
    this.ihash[2] = 0x3c6ef372;
    this.ihash[3] = 0xa54ff53a;
    this.ihash[4] = 0x510e527f;
    this.ihash[5] = 0x9b05688c;
    this.ihash[6] = 0x1f83d9ab;
    this.ihash[7] = 0x5be0cd19;
  }

  /* Transform a 512-bit message block */
  private sha256Transform() {
    let a: number = 0;
    let b: number = 0; 
    let c: number = 0;
    let d: number = 0; 
    let e: number = 0;
    let f: number = 0; 
    let g: number = 0; 
    let h: number = 0; 
    let T1: number = 0; 
    let T2: number = 0;
    let W : number[] = new Array(16);

    /* Initialize registers with the previous intermediate value */
    a = this.ihash[0];
    b = this.ihash[1];
    c = this.ihash[2];
    d = this.ihash[3];
    e = this.ihash[4];
    f = this.ihash[5];
    g = this.ihash[6];
    h = this.ihash[7];

    /* make 32-bit words */
    for (let i = 0; i < 16; i++) {
      W[i] = ((this.buffer[(i << 2) + 3]) |
        (this.buffer[(i << 2) + 2] << 8) | (this.buffer[(i << 2) + 1] << 16) | (this.buffer[i << 2] << 24));
    }
    
    for (let j = 0; j < 64; j++) {
      T1 = h + this.sha256Sigma1(e) + this.choice(e, f, g) + this.K256[j];
      if (j < 16) {
        T1 += W[j];
      } else {
        T1 += this.sha256Expand(W, j);
        T2 = this.sha256Sigma0(a) + this.majority(a, b, c);
        h = g;
        g = f;
        f = e;
        e = this.safeAdd(d, T1);
        d = c;
        c = b;
        b = a;
        a = this.safeAdd(T1, T2);
      }
    }

    /* Compute the current intermediate hash value */
    this.ihash[0] += a;
    this.ihash[1] += b;
    this.ihash[2] += c;
    this.ihash[3] += d;
    this.ihash[4] += e;
    this.ihash[5] += f;
    this.ihash[6] += g;
    this.ihash[7] += h;
  }

  /* Read the next chunk of data and update the SHA256 computation */
  private sha256Update(data : string, inputLen : number) {
    let i : number = 0;
    let index : number = 0; 
    let curpos : number = 0;
    /* Compute number of bytes mod 64 */
    index = ((this.count[0] >> 3) & 0x3f);
    let remainder : number = (inputLen & 0x3f);

    /* Update number of bits */
    if ((this.count[0] += (inputLen << 3)) < (inputLen << 3)) {
      this.count[1]++;
    }
    this.count[1] += (inputLen >> 29);

    /* Transform as many times as possible */
    for (i = 0; i + 63 < inputLen; i += 64) {
      for (let j : number = index; j < 64; j++) {
        this.buffer[j] = data.charCodeAt(curpos++);
      }
      this.sha256Transform();
      index = 0;
    }

    /* Buffer remaining input */
    for (let j = 0; j < remainder; j++) {
      this.buffer[j] = data.charCodeAt(curpos++);
    }

  }

  /* Finish the computation by operations such as padding */
  private sha256Final() {
    let index = ((this.count[0] >> 3) & 0x3f);
    this.buffer[index++] = 0x80;
    if (index <= 56) {
      for (let i = index; i < 56; i++) {
        this.buffer[i] = 0;
      }
    } else {
      for (let i = index; i < 64; i++) {
        this.buffer[i] = 0;
      }
      this.sha256Transform();

      for (let i = 0; i < 56; i++) {
        this.buffer[i] = 0;
      }

    }
    this.buffer[56] = (this.count[1] >>> 24) & 0xff;
    this.buffer[57] = (this.count[1] >>> 16) & 0xff;
    this.buffer[58] = (this.count[1] >>> 8) & 0xff;
    this.buffer[59] = this.count[1] & 0xff;
    this.buffer[60] = (this.count[0] >>> 24) & 0xff;
    this.buffer[61] = (this.count[0] >>> 16) & 0xff;
    this.buffer[62] = (this.count[0] >>> 8) & 0xff;
    this.buffer[63] = this.count[0] & 0xff;
    this.sha256Transform();
  }

  public sha256HexDigits = '0123456789abcdef';

  /* Split the internal hash values into an array of bytes */
  private sha256_encode_bytes() {
    let j = 0;
    let output : number[] = new Array(32);
    for (let i = 0; i < 8; i++) {
      output[j++] = ((this.ihash[i] >>> 24) & 0xff);
      output[j++] = ((this.ihash[i] >>> 16) & 0xff);
      output[j++] = ((this.ihash[i] >>> 8) & 0xff);
      output[j++] = (this.ihash[i] & 0xff);
    }
    return output;
  }

  /* Get the internal hash as a hex string */
  private sha256EncodeHex() : string {
    let output: string = '';
    for (let i = 0; i < 8; i++) {
      for (let j = 28; j >= 0; j -= 4) {
        output += this.sha256HexDigits.charAt((this.ihash[i] >>> j) & 0x0f);
      }

    }
    return output;
  }

  /* Main private: returns a hex string representing the SHA256 value of the
  given data */
  public sha256Digest(data : string) : string {
    this.sha256Init();
    this.sha256Update(data, data.length);
    this.sha256Final();
    return this.sha256EncodeHex();
  }

  public static getInstance(): HashUtils {
    if (HashUtils.instance == null) {
      HashUtils.instance = new HashUtils();
    }
    return HashUtils.instance;
  }

  public sha256DigestByMd(data: string): string {
    let md = cryptoFramework.createMd('SHA256');
    md.updateSync({ data: new Uint8Array(buffer.from(data, 'utf-8').buffer) });
    let mdOutput = md.digestSync();
    return buffer.from(mdOutput.data).toString('hex');
  }
}
